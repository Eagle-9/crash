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

enum MetaCharType
{
    NotMeta,
    Pipe,      // This is '|'
    Store,     // This is '>'
    StoreErr,  // This is '2>'
    Append,    // This is '>>'
    AppendErr, // This is '2>>'
};

enum TokenType
{
    Keyword,
    Argument,
    Internal, // Internal command
    External, // External command
    MetaChar
};

struct Token
{
    TokenType type;                                 // Type of token
    MetaCharType meta;                              // Meta character type
    std::string data;                               // String data
    int (*function_pointer)(int argc, char **argv); // Pointer to the function
};

struct KeywordEntry
{
    TokenType keyword;
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
        {"if", {Keyword, nullptr}},
        {"in", {Keyword, nullptr}},
        {"return", {Keyword, nullptr}},
        {"then", {Keyword, nullptr}},
        {"until", {Keyword, nullptr}},
        {"while", {Keyword, nullptr}},
        {"alias", {Internal, nullptr}},
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
        {"unalias", {Internal, nullptr}}};

/********************************************************************/
/*  Utility functions                                               */
/********************************************************************/

void print_prompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    std::cout << "CRASH " << std::string(cwd) << " " << PROMPT_NEW;
}

std::string kwtype_as_string(TokenType type)
{
    switch (type)
    {
    case Internal:
        return "internal";
    case External:
        return "external";
    case Keyword:
        return "keyword";
    case Argument:
        return "argument";
    case MetaChar:
        return "meta char";
    }
    return "";
}

std::vector<std::string> split_line(std::string inputString)
{
    std::vector<std::string> splitLine;
    std::string newSplit;
    // Check every character
    for (size_t i = 0; i < inputString.length(); i++)
    {
        // Check if quoted
        bool quoteLeft = false;
        bool quoteRight = false;
        // Make sure position is not at end or start of line.
        if (i > 0 && i < (inputString.length() - 1))
        {
            // Check to see if there is a quote left or right of current char.
            if (inputString[i - 1] == '"')
            {
                quoteLeft = true;
            }
            if (inputString[i + 1] == '"')
            {
                quoteRight = true;
            }
        }
        // We have an unquoted space, so we must split
        if ((!quoteLeft && !quoteRight) && inputString[i] == ' ')
        {
            if (!newSplit.empty())
            { // Check to make sure the line we are adding isn't empty
                splitLine.emplace_back(newSplit);
            }
            newSplit.clear();
        }
        else
        { // Don't need to split, so add
            newSplit = newSplit + inputString[i];
        }
    }
    // The above loop only adds an argument if there is a space, so we need this to get the inital arg.
    if (inputString != "")
    {
        splitLine.emplace_back(newSplit);
    }
    return splitLine;
}

MetaCharType check_meta(std::string inputString)
{
    // Check if quoted
    bool quoteLeft = false;
    bool quoteRight = false;
    // Check to see if string is quoted on left or right side
    if (inputString[0] == '"')
    {
        quoteLeft = true;
    }
    if (inputString[inputString.length() - 1] == '"')
    {
        quoteRight = true;
    }
    if (quoteLeft && quoteRight)
    {
        return NotMeta; // Not a metacharacter as it is quoted.
    }
    // Check if metacharacter
    if (inputString == "|")
    {
        return Pipe;
    }
    if (inputString == ">")
    {
        return Store;
    }
    if (inputString == ">>")
    {
        return Append;
    }
    if (inputString == "2>")
    {
        return StoreErr;
    }
    if (inputString == "2>>")
    {
        return AppendErr;
    }

    return NotMeta; // did not find a meta char
}

// used to kill children
void sigint_handler(int sig)
{
    exit(0);
}

/********************************************************************/
/*  Runtime functions                                               */
/********************************************************************/

std::string get_from_path(std::string command)
{
    const char *env_p = std::getenv("PATH");
    if (!env_p)
    {
        std::cout << "[WARN]: Failed to get $PATH!\n";
        return {};
    }

    // path to std::string
    std::string env_s = std::string(env_p);
    // get cwd
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    // prepend cwd to path
    std::stringstream stream(cwd + (':' + env_s));
    // for storing path segment (single path)
    std::string segment;

    // iterate over paths from $PATH
    while (std::getline(stream, segment, ':'))
    {
        // get path to test by appending arg[0] to the segment
        std::string test_path = segment + "/" + command;

        // check if the path is a valid file
        struct stat sb;
        if (stat(test_path.c_str(), &sb) == 0 && !(sb.st_mode & S_IFDIR))
        {
            return test_path;
        }
    }

    return {};
}

std::vector<char *> argv_from_tokens(std::vector<Token> tokens)
{
    std::vector<char *> argv;
    argv.reserve(tokens.size());
    for (size_t i = 0; i < tokens.size(); i++)
    {
        // anyone know a better way?
        argv.push_back(new char[tokens[i].data.size()]);
        strcpy(argv[i], tokens[i].data.c_str());
    }
    return argv;
}

void run_command(std::vector<Token> tokens, std::string filename, bool append, bool err)
{
    // needed to pass to fns
    std::vector<char *> argv = argv_from_tokens(tokens);

    // check token type
    if (tokens[0].type == Internal)
    {
        if (tokens[0].function_pointer)
        {
            tokens[0].function_pointer(argv.size(), argv.data());
        }
        else
        {
            std::cout << "[WARN]: Not yet implemented!\n";
        }
        // todo: free members of `argv`
    }
    else if (tokens[0].type == External)
    {
        std::string full_path = get_from_path(tokens[0].data);
        std::cout << full_path << std::endl;
        if (full_path.empty())
        {
            std::cout << "Command not found: " << tokens[0].data << "\n";
        }
        argv.erase(argv.begin());

        FILE* fd = nullptr;
        if (filename != "")
        {
            fd = fopen(filename.c_str(), append ? "a" : "w");
        }

        // create a child process
        pid_t child = fork();

        //  check if we're the child or the parent
        if (child == 0)
        {
            if (fd != nullptr)
            {
                int fd_to_override = err ? STDERR_FILENO : STDOUT_FILENO;
                if (dup2(fileno(fd), fd_to_override) < 0)
                {
                    std::perror("dup2 (stdout)");
                    std::exit(1);
                }
            }

            // we're the child
            // allow kill
            struct sigaction action;
            memset(&action, 0, sizeof(action));
            action.sa_handler = sigint_handler;
            sigaction(SIGINT, &action, NULL);

            // switch execution to new binary

            execv(full_path.c_str(), argv.data());

            std::cerr << "[!!] What are we doing here?!\n";
            exit(130);
        }
        else
        {
            // we're the parent
            // prevent kill while bearing children
            struct sigaction action;
            memset(&action, 0, sizeof(action));
            action.sa_handler = SIG_IGN;
            sigaction(SIGINT, &action, NULL);

            std::cout << "[*] waiting...\n";

            // wait for the child to finish running (or was cancelled by user)
            int status;
            waitpid(child, &status, 0);

            if (fd != nullptr) fclose(fd);

            // allow kill after children exit
            memset(&action, 0, sizeof(action));
            action.sa_handler = sigint_handler;
            sigaction(SIGINT, &action, NULL);
        }
    }
    else
    {
        std::cout << "[ERROR]: Command was not of type \"Internal\" or \"External\"!\n";
    }
}

std::vector<Token> lex(std::vector<std::string> splitLineToParse)
{
    std::vector<Token> tokens;
    for (std::string entry : splitLineToParse)
    {
        Token newToken;
        // Check to see if token is a meta character
        MetaCharType checkType = check_meta(entry);
        if (checkType != NotMeta)
        {
            newToken.type = MetaChar;
            newToken.meta = checkType;
        }
        // Check to see if in dictionary (internal or keyword)
        else if (dict.count(entry))
        { // The entry is in the dict
            newToken.type = dict.at(entry).keyword;
            if (newToken.type == Internal)
            { // The entry is is an internal function
                newToken.function_pointer = dict.at(entry).function_pointer;
                newToken.data = entry;
            }
            else
            { // The entry is internal keyword
                // TODO: For nick, this might need changed
                newToken.data = entry;
            }
        }
        // If previous token was internal, external, or argument it is an argument, otherwise external
        else if (tokens.size() != 0)
        {
            Token lastToken = tokens[tokens.size() - 1];
            if (lastToken.type == External || lastToken.type == Internal || lastToken.type == Argument)
            { // Is argument
                newToken.type = Argument;
                newToken.data = entry;
            }
            else
            { // Is external (last token was meta char or keyword, or is first)
                newToken.type = External;
                newToken.data = entry;
            }
        }
        else
        { // If there are no tokens, the current token must be external
            newToken.type = External;
            newToken.data = entry;
        }
        tokens.emplace_back(newToken);
    }
    return tokens;
}

void process(std::vector<Token> tokens)
{
    MetaCharType redirect_type = NotMeta;
    std::vector<Token> lhs;
    std::vector<Token> rhs;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        std::cout << kwtype_as_string(tokens[i].type) << "\n";
        if (tokens[i].type == MetaChar)
        {
            if (redirect_type != NotMeta)
                std::cout << "[WARN]: Multiple operators on one line are not supported!\n";
            else
            {
                redirect_type = tokens[i].meta;
                lhs.assign(tokens.begin(), tokens.begin() + i);
                rhs.assign(tokens.begin() + i + 1, tokens.end()); // +1 to skip meta
            }
        }
    }

    if (redirect_type == NotMeta)
    {
        run_command(tokens, {}, false, false);
        print_prompt();
        return;
    }

    bool do_append;
    switch(redirect_type)
    {
    case Store:
    case StoreErr:
        do_append = false;
        break;
    case Append:
    case AppendErr:
        do_append = true;
        break;
    }

    std::string filename = rhs[0].data.c_str();
    switch(redirect_type)
    {
    case NotMeta:
        std::perror("Something went terribly wrong!");
        break;
    case Store:
    case Append:
        run_command(lhs, filename, do_append, false);
        break;
    case AppendErr:
    case StoreErr:
        run_command(lhs, filename, do_append, true);
        break;
    case Pipe:
        std::cout << "[TODO]: pipe\n";
        return;
        break;
    }

    print_prompt();
}

void format_input(std::string line)
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

    std::vector<Token> tokens = lex(split_line(current_line));

    process(tokens);
    return;
}