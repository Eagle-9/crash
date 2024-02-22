

/**
 *
 *  impl.cc
 *      This file is for implementing "impl.hh"
 *
 */

#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
#include <impl.hh>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <sys/wait.h> // for wait()
#include <unistd.h>   // for exec()
#include <sys/stat.h>
#define KEYWORD "keyword"
#define INTERNAL "internal"
#define EXTERNAL "external"
#define HOME getenv("HOME")
#define HISTORY_FILE_NAME "/cd_history.txt"
#define HISTORY_FILE_PATH (std::string(HOME) + "/cd_history.txt").c_str()

//* state info

// if we're currently in a continuation
bool is_continuation = false;
// the current line
std::string current_line;

struct DictStruct
{
    std::string keyword;
    int (*function_pointer)(int argc, char **argv);
};

// used to kill children
void sigint_handler(int sig)
{
    exit(0);
}

// classification table
// the three classifications are
std::unordered_map<std::string, DictStruct> dict =
    {
        {"break", {KEYWORD, nullptr}},
        {"continue", {KEYWORD, nullptr}},
        {"do", {KEYWORD, nullptr}},
        {"else", {KEYWORD, nullptr}},
        {"elseif", {KEYWORD, nullptr}},
        {"end", {KEYWORD, nullptr}},
        {"endif", {KEYWORD, nullptr}},
        {"for", {KEYWORD, nullptr}},
        {"function", {KEYWORD, nullptr}},
        {"if", {KEYWORD, nullptr}},
        {"in", {KEYWORD, nullptr}},
        {"return", {KEYWORD, nullptr}},
        {"then", {KEYWORD, nullptr}},
        {"until", {KEYWORD, nullptr}},
        {"while", {KEYWORD, nullptr}},
        {"alias", {INTERNAL, nullptr}},
        {"bg", {INTERNAL, nullptr}},
        {"cd", {INTERNAL, builtin_cd}},
        {"eval", {INTERNAL, nullptr}},
        {"exec", {INTERNAL, nullptr}},
        {"exit", {INTERNAL, builtin_exit}},
        {"export", {INTERNAL, nullptr}},
        {"fc", {INTERNAL, nullptr}},
        {"fg", {INTERNAL, nullptr}},
        {"help", {INTERNAL, nullptr}},
        {"history", {INTERNAL, nullptr}},
        {"jobs", {INTERNAL, nullptr}},
        {"let", {INTERNAL, nullptr}},
        {"local", {INTERNAL, nullptr}},
        {"logout", {INTERNAL, nullptr}},
        {"read", {INTERNAL, nullptr}},
        {"set", {INTERNAL, nullptr}},
        {"shift", {INTERNAL, nullptr}},
        {"shopt", {INTERNAL, nullptr}},
        {"source", {INTERNAL, nullptr}},
        {"unalias", {INTERNAL, nullptr}}};

//* function implementation

// this takes no arguments, as it uses 'line' as it's input.
// 'line' should be the parsed line from 'parse()'
std::string process()
{
    std::string res;
    std::vector<std::string> args;
    std::vector<std::vector<char>> holder;
    std::vector<char *> argv;

    // get parsed line
    res = current_line;

    // Go through every character in the line, split them into args
    std::string tempArg;
    for (size_t i = 0; i < res.length(); i++)
    {
        if (!checkMetacharacter(res, i))
        { // If not a meta character, add to tempArg
            tempArg = tempArg + res[i];
        }
        else
        { // We hit a meta character, so we split the line. Add current arg to args, reset temp arg.
            args.emplace_back(tempArg);
            tempArg = "";
        }
    }
    // The above loop only adds an argument if there is a space, so we need this to get the inital arg.
    if (res != "")
    {
        args.emplace_back(tempArg);
    }

    // resize holder and argv to the args size
    holder.reserve(args.size());
    argv.reserve(args.size());

    /* Debug code to print out arguments. Can be removed without issue.*/
    for (size_t i = 0; i < args.size(); i++)
    {
        std::cout << "ARG[" << i << "]: " << args[i] << std::endl;
        ;
        holder.emplace_back(args[i].begin(), args[i].end());
        holder.back().push_back('\0');
        argv.push_back(holder.back().data());
    }

    // if the map returns a key
    if (dict.count(args[0]))
    {

        // get class from dictionary
        std::string lineClassName = dict.at(args[0]).keyword;
        if (dict.at(args[0]).function_pointer != nullptr)
        {
            dict.at(args[0]).function_pointer(args.size(), argv.data());
        }
        else
        {
            std::cout << "NOT YET IMPLEMENTED" << std::endl;
        }

        // append class to line
        res = res + " " + lineClassName;
    }
    else
    {

        if (const char *env_p = std::getenv("PATH"))
        {
            std::string env_s = std::string(env_p);
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            std::stringstream stream(env_s + ":" + cwd);
            std::string segment;
            bool found = false;

            while (std::getline(stream, segment, ':'))
            {
                std::string test_path = segment + "/" + args[0];

                struct stat sb;
                if (stat(test_path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR))
                {
                    std::cout << test_path << std::endl;

                    pid_t child = fork();

                    if (child == 0)
                    {

                        // allow kill
                        struct sigaction action;
                        memset(&action, 0, sizeof(action));
                        action.sa_handler = sigint_handler;
                        sigaction(SIGINT, &action, NULL);

                        execv(test_path.c_str(), argv.data());
                    }
                    else
                    {
                        // prevent kill while bearing children
                        struct sigaction action;
                        memset(&action, 0, sizeof(action));
                        action.sa_handler = SIG_IGN;
                        sigaction(SIGINT, &action, NULL);

                        int status;
                        waitpid(child, &status, 0);

                        // allow kill after children exit
                        memset(&action, 0, sizeof(action));
                        action.sa_handler = sigint_handler;
                        sigaction(SIGINT, &action, NULL);
                    }

                    found = true;
                    break;
                }
            }

            if (!found)
            {
                std::cout << "Failed to find command: " << args[0] << "\n";
            }
        }
        // not in dictionary
        res = res + " external";

    } // end if

    // add prompt to end of response
    res += "\n";
    res += getNewPrompt();

    // clear current line + return
    current_line.clear();
    return res;
}

// commented in header
std::string parse(std::string line)
{
    // if there's a blank line
    if (line.length() == 0)
    {
        return getNewPrompt();
    }

    // to store the comment start location
    size_t comment_start = std::string::npos;
    // for char in line, search for a comment start location
    for (size_t i = 0; i < line.length(); i++)
    {
        if (
            line[i] == '#' && // we have a pound and one of the following
            (
                i == 0 ||                                      // the pound is the first character; full-line comment
                (line[i - 1] != '\'' && line[i - 1] != '"') || // the pound doesn't start with a quote
                (line[i + 1] != '\'' && line[i + 1] != '"')    // the pound doesn't end with a quote
                ))
        {
            // we found a comment start location
            comment_start = i;
            break;
        }
    }

    // if we found a comment start location
    if (comment_start != std::string::npos)
    {
        // if it's a full line comment, skip the line
        if (comment_start == 0)
        {
            if (!is_continuation)
            {
                return getNewPrompt();
            }
            line = "";
        }
        else
        {
            line = line.substr(0, comment_start);
        }
    }
    // Dylan: Remove extra whitespace from the line.
    std::string tempLine;
    bool encounteredFirstChar = false;
    for (size_t i = 0; i < line.length(); i++)
    {
        if (line[i] == ' ' || line[i] == '\t')
        { // Check and see if we have found a space or tab.
            // First check to make sure we are not at the end of the line and we have had a character
            if (i + 1 < line.length() && encounteredFirstChar)
            {
                // If the next character is not a space/tab, we can add a space.
                if (line[i + 1] != ' ' && line[i + 1] != '\t')
                {
                    tempLine += ' ';
                }
            }
        }
        else
        { // The character was not a space or tab, so just add it
            tempLine += line[i];
            encounteredFirstChar = true;
        }
    }
    line = tempLine;

    // if there's a continuation
    if (line != "" && line[line.length() - 1] == '\\')
    {
        line.pop_back();
        is_continuation = true;
    }
    else
    {
        is_continuation = false;
    }

    // append parsed to current_line
    current_line += line;
    // is this a continuation?
    if (is_continuation)
    {
        return PROMPT_CNT;
    }
    // not a continuation, as we would've returned
    return process();
}

// Check if there is a meta character in the given string at the given position.
bool checkMetacharacter(std::string inputString, size_t position)
{
    // Check if quoted
    bool quoteLeft = false;
    bool quoteRight = false;
    // Make sure position is not at end or start of line.
    if (position > 0 && position < (inputString.length() - 1))
    {
        // Check to see if there is a quote left or right of current char.
        if (inputString[position - 1] == '"')
        {
            quoteLeft = true;
        }
        if (inputString[position + 1] == '"')
        {
            quoteRight = true;
        }
    }
    if (quoteLeft && quoteRight)
    {
        return false; // Not a metacharacter as it is quoted.
    }
    // Check if metacharacter
    std::string metaCharacters = "|&;()<> \\";
    char indivChar = inputString[position];
    for (size_t i = 0; i < metaCharacters.length(); i++)
    {
        if (indivChar == metaCharacters[i])
        {
            return true; // meta char found
        }
    }
    return false; // did not find a meta char
}
std::string getNewPrompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    return "CRASH " + std::string(cwd) + " " + PROMPT_NEW;
}

// commented in header
std::string _get_current()
{
    return current_line;
}

// exit command
int builtin_exit(int argc, char **argv)
{
    exit(0);
}

// CD COMMANDS

int builtin_cd(int argc, char **argv)
{
    // cd function

    std::string key = "";
    if (argc >= 2)
    {                  // checks to make sure there is enough arguments
        key = argv[1]; // sets key to flag
    }

    // if argument is -{n}, convert to string to select from table
    // note: this works primarily because of short circuit evaluation
    if (argc >= 2 && isdigit(argv[1][1]) && argv[1][0] == '-')
    {
        key = "-{n}";
    }

    // table to store all flags in
    std::unordered_map<std::string, int (*)(int argc, char ** argv)> cd_table; // key = int, value is array of strings. all funcs must be formatted like 'void funcName(int argc, std::string* argv)'

    cd_table["-h"] = cd_help_message;         // displays a simple help message
    cd_table["-H"] = cd_help_message;         // displays a full help message
    cd_table["-l"] = cd_print_history;        // Display a history list
    cd_table["-{n}"] = cd_nth_history;        // Change current directory to nth element
    cd_table["-c"] = cd_clear_history;        // clean the directory history
    cd_table["-s"] = cd_print_unique_history; // suppress the directory history

    // make sure key is in table
    if (cd_table.find(key) != cd_table.end())
    {
        // access table
        return cd_table[key](argc, argv);
    }
    else if (key[0] != '-')
    {
        if (chdir(key.c_str()) != 0)
        {
            std::cout << "err\n";
            return 1;
        }
        else
        {
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            cd_write_history_file(std::string(cwd));
        }
    }
    else
    {
        // not in table
        std::cout << "The flag " << key << " is not an argument of cd" << std::endl;
        return 1;
    }

    return 0;
}


int cd_help_message(int argc, char ** argv)

{

    // simple help message
    std::string simpleHelp = "To change directory, input 'cd DIR' where DIR is the desired directory's address";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'cd'\n\ncd [-h] [-H] [-l [{n}]] [-{n}] [-c] [-s] (DIR)\n\nGeneral Use\n\nChange the current directory to DIR. The default DIR is the current directory so that it is identical to the pwd command in typical Linux shells\n\nArguments\n\n-h : Display simple help message\n-H : Display full help message\n-l [{n}] : Display the history list of the current directories with serial numbers. With the optional N it displays only the last N entries\n-{n} : Change the current directory to the n-th entry in the history list\n-c : Clean the directory history\n-s : Suppress the directory history by eliminating duplicated directories. The order of the history must be maintained. The latest entry must be kept for same directories";

    // differentiate between simple and complex help message
    if (strcmp(argv[1], "-h") == 0)
    {
        std::cout << simpleHelp << std::endl; // simple help message
    }
    else if (strcmp(argv[1], "-H") == 0)
    {
        std::cout << fullHelp << std::endl; // full help message
    }
    else
    {
        std::cout << "not a known command. Did you mean cd -h or cd -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}

int cd_history_length()
{
    // open up the file
    std::ifstream historyFile;
    historyFile.open(HISTORY_FILE_PATH);

    // check that the file is open
    if (historyFile.fail())
    {
        cd_create_history_file();
        historyFile.open(HISTORY_FILE_PATH);
    }

    // lines will be our return value
    int lines = 0;

    // line will temporarily hold each of the strings
    std::string line;

    // use a while loop to find the end of the file
    while (!historyFile.eof())
    {
        // get the current line to get to the next line
        getline(historyFile, line);
        lines++;
    }

    return lines;
}

int cd_print_history(int argc, char **argv)
{
    // if we have an n for number of lines, we run the other function
    if (argc >= 3 && isdigit(argv[2][0]))
    {
        cd_print_history(atoi(argv[2]));
    }
    else
    {
        // define and open the history file
        std::ifstream historyFile;
        historyFile.open(HISTORY_FILE_PATH);

        // check that the file is open
        if (historyFile.fail())
        {
            cd_create_history_file();
            historyFile.open(HISTORY_FILE_PATH);
        }

        // define the line that we will use to get the current line
        std::string line;

        // print out every line in the file
        while (!historyFile.eof())
        {
            getline(historyFile, line);
            std::cout << line << std::endl;
        }
        historyFile.close();
    }
  return 0;
}

// this overload will print out the last n lines
// it is to be called from the normal cd_print_history
void cd_print_history(int n)
{
    // get the number of lines in the file
    int totalLen = cd_history_length();

    // define and open the history file
    std::ifstream historyFile;
    historyFile.open(HISTORY_FILE_PATH);

    // define the line that we will use to get the current line
    std::string line;

    // move down the file totalLen - n spaces
    for (int i = 0; i < (totalLen - n - 1); i++)
    {
        getline(historyFile, line);
    }

    while (!historyFile.eof())
    {
        getline(historyFile, line);
        std::cout << line << std::endl;
    }
    historyFile.close();
}

// if the file was not created, recreate it here
void cd_create_history_file()
{
    std::ofstream writeFile;
    writeFile.open(HISTORY_FILE_PATH);
    if (writeFile.fail())
    {
        std::cout << "CREATE HISTORY FILE FAILED" << std::endl;
    }
    writeFile.close();
}

void cd_write_history_file(const std::string dir)
{
    int serialNum = cd_history_length();
    std::ofstream historyFile;
    historyFile.open(HISTORY_FILE_PATH, std::ios::app);
    historyFile << serialNum << ":" << dir << std::endl;
    historyFile.close();
}

int cd_clear_history(int argc, char **argv)
{
    // delete the history file
    std::remove(HISTORY_FILE_PATH);

    // create a new history file
    cd_create_history_file();
  
  return 0;
}

int cd_nth_history(int argc, char **argv)
{
    int n = std::abs(atoi(argv[1])); //We need to get the absolute values of this, because the argument has a negative sign lol.
    // checks that everything is valid
    if (argc <= 2 && isdigit(argv[1][1]) && n < cd_history_length())
    {
        // open the file
        std::ifstream historyFile;
        historyFile.open(HISTORY_FILE_PATH);

        // find the directory we need to change to
        std::string line;
        for (int i = 0; i < n; i++)
        {
            getline(historyFile, line);
        }

        // find the location of the : character, which marks the serial number and the path
        int loc = line.find(':');

        // start from the loc + 1 location to get the full path to change to
        std::string dir = line.substr(loc + 1);

        // if the chdir failed, report error. otherwise, log the cwd
        if (chdir(dir.c_str()) != 0)
        {
            std::cout << "err: " << dir << std::endl;
        }
        else
        {
            // log the history
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            cd_write_history_file(std::string(cwd));
        }
    }
    else
    {
        std::cout << "INVALID NUMBER" << std::endl;
        return 1;
    }
    return 0;
}

int cd_print_unique_history(int argc, char **argv)
{
    // create a vector to temporarily store the values
    std::vector<std::string> tempLine;

    // open up the history file
    std::ifstream historyFile;
    historyFile.open(HISTORY_FILE_PATH);

    // check that the history file is open
    if (historyFile.fail())
    {
        cd_create_history_file();
        historyFile.open(HISTORY_FILE_PATH);
    }

    std::string line;
    while (!historyFile.eof())
    {
        bool pathExists = false;
        // get the line in the file
        getline(historyFile, line);
        int loc = line.find(':');
        std::string filePath = line.substr(loc + 1);

        // look through the tempLine vector to see if the path exists
        for (size_t i = 0; i < tempLine.size(); i++)
        {
            if (strcmp(filePath.c_str(), tempLine.at(i).c_str()) == 0)
            {
                pathExists = true;
                break;
            }
        }

        // if the path does not exist, print out the line and add it to the vector
        if (!pathExists)
        {
            std::cout << line << std::endl;
            tempLine.push_back(filePath);
        }
    }
    return 0;
}
