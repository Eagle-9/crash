#include <crash.hh>
#define CD_HISTORY_FILE_PATH (std::string(HOME) + "/cd_history.txt").c_str()
#define PRINT_CD GREEN << "[CD]" << RESET

int cd_help_message(int argc, char **argv)
{

    // simple help message
    std::string simpleHelp = "cd: cd [-h] [-H] [-l [{n}]] [-{n}] [-c] [-s] (DIR)";

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
        std::cerr << PRINT_CD << PRINT_ERROR << ": Not a known command. Did you mean cd -h or cd -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}

// if the file was not created, recreate it here
void cd_create_history_file()
{
    std::ofstream writeFile;
    writeFile.open(CD_HISTORY_FILE_PATH);
    if (writeFile.fail())
    {
        std::cerr << PRINT_CD << PRINT_ERROR << ": Failed to create history file" << std::endl;
    }
    writeFile.close();

    if(crash_debug)
        std::cout << PRINT_CD << PRINT_DEBUG << ": Created cd history file" << std::endl;
}

int cd_history_length()
{
    // open up the file
    std::ifstream historyFile;
    historyFile.open(CD_HISTORY_FILE_PATH);

    // check that the file is open
    if (historyFile.fail())
    {
        cd_create_history_file();
        historyFile.open(CD_HISTORY_FILE_PATH);
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

// this overload will print out the last n lines
// it is to be called from the normal cd_print_history
void cd_print_history(int n)
{
    // get the number of lines in the file
    int totalLen = cd_history_length();

    // define and open the history file
    std::ifstream historyFile;
    historyFile.open(CD_HISTORY_FILE_PATH);

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
        historyFile.open(CD_HISTORY_FILE_PATH);

        // check that the file is open
        if (historyFile.fail())
        {
            cd_create_history_file();
            historyFile.open(CD_HISTORY_FILE_PATH);
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

void cd_write_history_file(const std::string dir)
{
    int serialNum = cd_history_length();
    std::ofstream historyFile;
    historyFile.open(CD_HISTORY_FILE_PATH, std::ios::app);
    historyFile << serialNum << ":" << dir << std::endl;
    historyFile.close();
}

int cd_clear_history(int argc, char **argv)
{
    // delete the history file
    std::remove(CD_HISTORY_FILE_PATH);

    // create a new history file
    cd_create_history_file();

    return 0;
}

int cd_nth_history(int argc, char **argv)
{
    int n = std::abs(atoi(argv[1])); // We need to get the absolute values of this, because the argument has a negative sign lol.
    // checks that everything is valid
    if (argc <= 2 && isdigit(argv[1][1]) && n < cd_history_length())
    {
        // open the file
        std::ifstream historyFile;
        historyFile.open(CD_HISTORY_FILE_PATH);

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
            std::cerr << PRINT_CD << PRINT_ERROR << ": Failed to change directory: " << dir << std::endl;
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
        std::cerr << PRINT_CD << PRINT_ERROR << ": INVALID NUMBER" << std::endl;
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
    historyFile.open(CD_HISTORY_FILE_PATH);

    // check that the history file is open
    if (historyFile.fail())
    {
        cd_create_history_file();
        historyFile.open(CD_HISTORY_FILE_PATH);
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
    std::unordered_map<std::string, int (*)(int argc, char **argv)> cd_table; // key = int, value is array of strings. all funcs must be formatted like 'void funcName(int argc, std::string* argv)'

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
            std::cerr << PRINT_CD << PRINT_ERROR << ": Invalid Directory: " << key << "\n";
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
        std::cerr << PRINT_CD << PRINT_ERROR << ": The flag " << key << " is not an argument of cd" << std::endl;
        return 1;
    }

    return 0;
}