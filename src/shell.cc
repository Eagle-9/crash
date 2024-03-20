#include <crash.hh>

/********************************************************************/
/*  State initialization                                            */
/********************************************************************/

// if we're currently in a continuation
bool is_continuation = false;
// the current line
std::string current_line;

/********************************************************************/
/*  Type/Data initialization                                        */
/********************************************************************/

enum KeywordType
{
    Keyword,
    Internal,
    External
};

struct KeywordEntry
{
    KeywordType keyword;
    int (*function_pointer)(int argc, char **argv);
};

std::unordered_map<std::string, KeywordEntry> dict =
    {
        {"break", {Keyword, nullptr}},
        {"continue", {Keyword, nullptr}},
        {"do", {Keyword, nullptr}},
        {"else", {Keyword, nullptr}},
        {"elseif", {Keyword, nullptr}},
        {"end", {Keyword, nullptr}},
        {"endif", {Keyword, nullptr}},
        {"for", {Keyword, nullptr}},
        {"function", {Keyword, nullptr}},
        {"if", {Keyword, keyword_if}},
        {"in", {Keyword, nullptr}},
        {"return", {Keyword, nullptr}},
        {"then", {Keyword, nullptr}},
        {"until", {Keyword, nullptr}},
        {"while", {Keyword, nullptr}},
        {"alias", {Internal, builtin_alias}},
        {"bg", {Internal, nullptr}},
        {"cd", {Internal, builtin_cd}},
        {"eval", {Internal, nullptr}},
        {"exec", {Internal, nullptr}},
        {"exit", {Internal, builtin_exit}},
        {"export", {Internal, nullptr}},
        {"fc", {Internal, nullptr}},
        {"fg", {Internal, nullptr}},
        {"help", {Internal, nullptr}},
        {"history", {Internal, builtin_history}},
        {"jobs", {Internal, nullptr}},
        {"let", {Internal, nullptr}},
        {"local", {Internal, nullptr}},
        {"logout", {Internal, nullptr}},
        {"read", {Internal, nullptr}},
        {"set", {Internal, nullptr}},
        {"shift", {Internal, nullptr}},
        {"shopt", {Internal, nullptr}},
        {"source", {Internal, nullptr}},
        {"unalias", {Internal, builtin_unalias}}};

std::unordered_map<std::string, std::string> aliases = {{"test", "history -69"}};

/********************************************************************/
/*  Utility functions                                               */
/********************************************************************/

void print_prompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    std::cout << "CRASH " << std::string(cwd) << " " << PROMPT_NEW;
}

std::string kwtype_as_string(KeywordType type)
{
    switch (type)
    {
    case Internal:
        return "internal";
    case External:
        return "external";
    case Keyword:
        return "keyword";
    }
    return "";
}

bool check_meta(std::string inputString, size_t position)
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

// used to kill children
void sigint_handler(int sig)
{
    exit(0);
}

/********************************************************************/
/*  Runtime functions                                               */
/********************************************************************/

void run_external_fn(std::string *res, std::vector<std::string> args, std::vector<char *> argv)
{
    // not in dictionary
    res->append(" ");
    res->append(kwtype_as_string(External));

    const char *env_p = std::getenv("PATH");
    if (!env_p)
        return;

    // path to std::string
    std::string env_s = std::string(env_p);
    // get cwd
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    // prepend cwd to path
    std::stringstream stream(cwd + (':' + env_s));
    // for storing path segment (single path)
    std::string segment;
    // if we found the command
    bool found = false;

    // iterate over paths from $PATH
    while (std::getline(stream, segment, ':'))
    {
        // get path to test by appending arg[0] to the segment
        std::string test_path = segment + "/" + args[0];

        // check if the path is a valid file
        struct stat sb;
        if (stat(test_path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR))
        {

            // create a child process
            pid_t child = fork();

            //  check if we're the child or the parent
            if (child == 0)
            {
                // we're the child

                // allow kill
                struct sigaction action;
                memset(&action, 0, sizeof(action));
                action.sa_handler = sigint_handler;
                sigaction(SIGINT, &action, NULL);

                // switch execution to new binary
                execv(test_path.c_str(), argv.data());
            }
            else
            {
                // we're the parent

                // prevent kill while bearing children
                struct sigaction action;
                memset(&action, 0, sizeof(action));
                action.sa_handler = SIG_IGN;
                sigaction(SIGINT, &action, NULL);

                // wait for the child to finish running (or was cancelled by user)
                int status;
                waitpid(child, &status, 0);

                // allow kill after children exit
                memset(&action, 0, sizeof(action));
                action.sa_handler = sigint_handler;
                sigaction(SIGINT, &action, NULL);
            }

            // mark found
            found = true;
            break;
        }
    }

    // print if not found
    if (!found)
    {
        std::cout << "ERROR: Failed to find command: " << args[0] << "\n";
    }
}

void process()
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
        if (!check_meta(res, i))
        { // If not a meta character, add to tempArg
            tempArg = tempArg + res[i];
        }
        else
        { // We hit a meta character, so we split the line. Add current arg to args, reset temp arg.
            if (!tempArg.empty())
            {
                args.emplace_back(tempArg);
            }
            tempArg.clear();
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

    for (size_t i = 0; i < args.size(); i++)
    {
        holder.emplace_back(args[i].begin(), args[i].end());
        holder.back().push_back('\0');
        argv.push_back(holder.back().data());
    }

    // if the map returns a key
    if (dict.count(args[0]))
    {

        // get class from dictionary
        std::string lineClassName = kwtype_as_string(dict.at(args[0]).keyword);
        if (dict.at(args[0]).function_pointer != nullptr)
        {
            dict.at(args[0]).function_pointer(args.size(), argv.data());
        }
        else
        {
            std::cout << "NOT YET IMPLEMENTED\n";
        }

        // append class to line
        res = res + " " + lineClassName;
    }
    else if (aliases.count(args[0]))
    {
        parse(aliases.at(args[0]));
    }
    else
    {
        run_external_fn(&res, args, argv);
    } // end if
    std::cout << res << "\n";
    print_prompt();
}

void parse(std::string line)
{
    history_write_history_file(line);
    // clear the current line before parsing
    current_line.clear();

    // if there's a blank line, return prompt
    if (line.length() == 0)
    {
        print_prompt();
        return;
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
                print_prompt();
                return;
            }
            line = "";
        }
        else
        {
            line = line.substr(0, comment_start);
        }
    }

    // remove extra whitespace from the line.
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
        std::cout << PROMPT_CNT << "\n";
        return;
    }

    // not a continuation, as we would've returned
    process();
    return;
}

int keyword_if(int argc, char** argv) {
  //conditional if statements

  std::vector<std::string> args;

  //this stores the pairs of if, then statements
  std::vector<std::vector<std::string>> conditionals;

  //print out all of the arguments to test
  for (int i = 0; i < argc; i++) {
    //convert char** to string to make it easier to work with
    std::string tempStr = argv[i];
    args.push_back(tempStr);
    std::cout << args[i] << std::endl;
  }

    std::vector<std::string> tempVec;
    
    //parser, counters to keep track of if inside an if statement
    int ifCounter = 0;
    
    std::cout << "args --" << std::endl;
    for (unsigned int i = 0; i < args.size(); i++) {
        std::cout << args[i] << std::endl;
    }
    
    std::cout << std::endl << "parsing --" << std::endl;
    //for each word in the vector
    for (unsigned int i = 0; i < args.size(); i++) {
        
        if (args[i] == "if") {
            //increase ifCounter
            ifCounter++;
            if(ifCounter > 1) {
                tempVec.push_back(args[i]);
            }
        } else if (args[i] == "then") {
            //done counting variables
            if(ifCounter <= 1) {
                //in sub if, ignore keywords
                conditionals.push_back(tempVec);
                tempVec.clear();
            } else {
                //add to vector arguments
                tempVec.push_back(args[i]);
            }
        } else if (args[i] == "elseif") {
            //new statement
            if(ifCounter <= 1) {
                conditionals.push_back(tempVec);
                tempVec.clear();
                ifCounter++;
            } else {
                //end of sub if
                tempVec.push_back(args[i]);
            }
        } else if (args[i] == "else") {
            //last statement always happens
            if(ifCounter <= 1) {
                conditionals.push_back(tempVec);
                tempVec.clear();
                
                //add empty vector to check for when evaluating
                conditionals.push_back(tempVec);
            } else {
                //end of sub if
                tempVec.push_back(args[i]);
            }
            
        } else if (args[i] == "endif") {
            //end of the statement
            if(ifCounter <= 1) {
                conditionals.push_back(tempVec);
                tempVec.clear();
            } else {
                //end of sub if
                tempVec.push_back(args[i]);
            }
            ifCounter--;
        } else if (args[i] == "[") {
            //begining of elseif
            if(ifCounter <= 1) {
                ifCounter--;   
            } else {
                tempVec.push_back(args[i]);
            }
        } else if (args[i] == "]") {
            if(ifCounter <= 1) {
                ifCounter--;   
            } else {
                tempVec.push_back(args[i]);
            }
        } else {
            //not a keyword, so a command
            tempVec.push_back(args[i]);
        }
    }
    
    std::cout << std::endl << "conditionals --" << std::endl;
    for (unsigned long int j = 0; j < conditionals.size(); j++) {
        std::cout << j << ": ";
        for (unsigned int i = 0; i < conditionals[j].size(); i++) {
            std::cout << conditionals[j][i] << ",";
        }
        std::cout << std::endl;
    }

  //loop through each conditional with dictionary
  //return exit status

  return 0;
}
