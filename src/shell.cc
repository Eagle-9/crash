#include <crash.hh>
#define PRINT_SHELL GREEN << "[SHELL]" << RESET

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
    Redirect,  // This is '<'
    StoreErr,  // This is '2>'
    Append,    // This is '>>'
    AppendErr, // This is '2>>'
};

struct Token
{
    TokenType type;                                 // Type of token
    MetaCharType meta;                              // Meta character type
    std::string data;                               // String data
    int (*function_pointer)(int argc, char **argv); // Pointer to the function
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
        {"help", {Internal, builtin_help}},
        {"history", {Internal, builtin_history}},
        {"jobs", {Internal, nullptr}},
        {"let", {Internal, nullptr}},
        {"local", {Internal, nullptr}},
        {"logout", {Internal, nullptr}},
        {"read", {Internal, nullptr}},
        {"set", {Internal, builtin_set}},
        {"shift", {Internal, nullptr}},
        {"shopt", {Internal, nullptr}},
        {"source", {Internal, nullptr}},
        {"unalias", {Internal, builtin_unalias}}};

std::unordered_map<std::string, std::string> aliases = {};
std::unordered_map<std::string, std::string> set = {};
bool crash_debug = false;
bool crash_exit_on_err = false;
/********************************************************************/
/*  Utility functions                                               */
/********************************************************************/

void print_prompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    if (crash_debug)
        std::cout << CYAN << "DEBUG_" << RESET;

    std::cout << MAGENTA << "CRASH " << BLUE << std::string(cwd) << RESET << " " << PROMPT_NEW;
}
std::vector<std::string> wildCardMatch(std::string wildCard) // Match wildcard to file in current directory
{
    glob_t globResult;
    std::vector<std::string> result;
    int returnValue = glob(wildCard.c_str(), GLOB_TILDE, nullptr, &globResult);
    if (returnValue == 0)
    {
        for (size_t i = 0; i < globResult.gl_pathc; i++)
        {
            result.emplace_back(globResult.gl_pathv[i]);
        }
        globfree(&globResult);
        return result;
    }
    if (returnValue == GLOB_NOMATCH)
    {
        std::cerr << PRINT_SHELL << PRINT_ERROR << ": No match found for wildcard: " << wildCard << std::endl;
        return result;
    }
    else
    {
        std::cerr << PRINT_SHELL << PRINT_ERROR << ": Failed glob wildcard search: " << returnValue << std::endl;
        return result;
    }
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
bool isLocationInStringQuoted(std::string inputString, size_t locationToCheck)
{
    std::vector<int> quoteIndexs;
    bool quoted = false;
    // Find all quoted sections
    for (size_t i = 0; i < inputString.length(); i++)
    {
        if (inputString[i] == '"')
        {
            quoteIndexs.emplace_back(i);
        }
    }
    // If there is an odd number of quotes, remove the last one from the index, as it won't be quotping anything. Ex: "hello world" "
    if (quoteIndexs.size() % 2 == 1)
    {
        quoteIndexs.pop_back();
    }

    // Check if character is quoted
    for (size_t indexRangeCount = 0; indexRangeCount < quoteIndexs.size() / 2; indexRangeCount++)
    {
        size_t lowerBound = quoteIndexs[indexRangeCount * 2];
        size_t upperBound = quoteIndexs[(indexRangeCount * 2) + 1];
        // If character is inside of bounds
        if ((locationToCheck > lowerBound) && (locationToCheck < upperBound))
        { // character is quoted
            quoted = true;
        }
    }
    return quoted;
}

std::vector<std::string> split_line(std::string inputString)
{
    std::vector<std::string> splitLine;
    std::string newSplit;

    // Check every character
    for (size_t i = 0; i < inputString.length(); i++)
    {
        // We have an unquoted space, so we must split
        if (!isLocationInStringQuoted(inputString, i) && inputString[i] == ' ')
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

std::vector<std::string> findReplaceAlias(std::vector<std::string> inputVector)
{
    // Check if there is unalias, if so, do not replace!
    for (size_t i = 0; i < inputVector.size(); i++)
    {
        if (inputVector.at(i) == "unalias")
        {
            return inputVector;
        }
    }

    std::vector<std::string> outputVector;
    // Check line for aliases, if so, replace
    for (size_t i = 0; i < inputVector.size(); i++)
    { // For every entry
        if (aliases.find(inputVector[i]) != aliases.end())
        { // We found an alias, so replace the entry with the alias
            for (std::string entry : split_line(aliases.at(inputVector[i])))
            {
                outputVector.emplace_back(entry);
            }
        }
        else
        { // We didn't find an alias, append
            outputVector.emplace_back(inputVector[i]);
        }
    }
    return outputVector;
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
    if (inputString == "<")
    {
        return Redirect;
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
        std::cerr << PRINT_SHELL << PRINT_ERROR << ": Failed to get $PATH!\n";
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

void run_command(std::vector<Token> tokens, int outfd, int errfd, int infd)
{
    // needed to pass to fns
    std::vector<char *> argv = argv_from_tokens(tokens);

    // check token type
    if (tokens[0].type == Internal)
    {
        if (tokens[0].function_pointer)
        {
            int error = tokens[0].function_pointer(argv.size(), argv.data());

            if (crash_exit_on_err && error != 0)
            {
                std::cerr << PRINT_SHELL << PRINT_ERROR << ": command failed." << std::endl;
                exit(error);
            }
        }
        else
        {
            std::cout << PRINT_SHELL << PRINT_WARN << ": Not yet implemented!\n";
        }
        // todo: free members of `argv`
    }
    else if (tokens[0].type == External)
    {
        std::string full_path = get_from_path(tokens[0].data);
        if (crash_debug)
        {
            std::cout << PRINT_SHELL << PRINT_DEBUG << ": External Path: " << full_path << std::endl;
        }

        if (full_path.empty())
        {
            std::cerr << PRINT_SHELL << PRINT_ERROR << ": Command not found: " << tokens[0].data << "\n";
            return;
        }
        // argv.erase(argv.begin()); This is bad!

        // create a child process
        pid_t child = fork();

        //  check if we're the child or the parent
        if (child == 0)
        {
            if (outfd != -1 && dup2(outfd, STDOUT_FILENO) < 0)
            {
                std::perror("dup2 (stdout)");
                std::exit(1);
            }
            if (errfd != -1 && dup2(errfd, STDERR_FILENO) < 0)
            {
                std::perror("dup2 (stderr)");
                std::exit(1);
            }
            if (infd != -1 && dup2(infd, STDIN_FILENO) < 0)
            {
                std::perror("dup2 (stdin)");
                std::exit(1);
            }

            // we're the child
            // allow kill
            struct sigaction action;
            memset(&action, 0, sizeof(action));
            action.sa_handler = sigint_handler;
            sigaction(SIGINT, &action, NULL);

            // switch execution to new binary

            argv.emplace_back(nullptr); // You might want this in a different spot? Maybe in argv_from_tokens? Idk?
            execv(full_path.c_str(), argv.data());

            std::cerr << PRINT_SHELL << PRINT_ERROR << ": Child process failure!\n";
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

            // allow kill after children exit
            memset(&action, 0, sizeof(action));
            action.sa_handler = sigint_handler;
            sigaction(SIGINT, &action, NULL);
        }
    }
    else
    {
        std::cerr << PRINT_SHELL << PRINT_ERROR << ": Command was not of type \"Internal\" or \"External\"!\n";
    }
}

std::vector<Token> lex(std::vector<std::string> inputLine)
{

    // Check line for wildcards, append selections
    std::vector<std::string> splitWildCardLineToParse;
    for (std::string entry : inputLine)
    {
        // Check to see if token has a wildcard that needs to be converted
        size_t foundQuestionMark = entry.find('?');
        size_t foundAsterisk = entry.find('*');
        size_t foundLeftBracket = entry.find('[');
        size_t foundRightBracket = entry.find(']');
        // Convert entry to wildcard if needed
        if (foundQuestionMark != std::string::npos || foundAsterisk != std::string::npos)
        { // We found a question mark
            for (std::string wildCardResult : wildCardMatch(entry))
            {
                splitWildCardLineToParse.emplace_back(wildCardResult);
            }
        }
        else if (foundLeftBracket != std::string::npos && foundRightBracket != std::string::npos)
        { // We found a bracket pair
            if (foundLeftBracket < foundRightBracket)
            { // Make sure the right bracket is actually right of the left one
                for (std::string wildCardResult : wildCardMatch(entry))
                {
                    splitWildCardLineToParse.emplace_back(wildCardResult);
                }
            }
        }
        else
        { // No wildcards in string
            splitWildCardLineToParse.emplace_back(entry);
        }
    }

    // Start tokenization
    std::vector<Token> tokens;
    for (std::string entry : splitWildCardLineToParse)
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
            {                          // The entry is is an internal function
                newToken.data = entry; // This is really not needed, but it's nice for debugging, to see the command name
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
        if (tokens[i].type == MetaChar)
        {
            if (redirect_type != NotMeta)
                std::cout << PRINT_SHELL << PRINT_WARN << ": Multiple operators on one line are not supported!\n";
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
        run_command(tokens, -1, -1, -1);
        print_prompt();
        return;
    }

    char open_mode;
    switch (redirect_type)
    {
    case Store:
    case StoreErr:
        open_mode = 'w';
        break;
    case Append:
    case AppendErr:
        open_mode = 'a';
        break;
    case Redirect:
        open_mode = 'r';
        break;
    }

    std::string filename = rhs[0].data.c_str();
    FILE *fd = nullptr;
    switch (redirect_type)
    {
    case NotMeta:
        std::perror("Something went terribly wrong!");
        break;
    case Store:
    case Append:
        fd = fopen(filename.c_str(), &open_mode);
        run_command(lhs, fileno(fd), -1, -1);
        fclose(fd);
        break;
    case AppendErr:
    case StoreErr:
        fd = fopen(filename.c_str(), &open_mode);
        run_command(lhs, -1, fileno(fd), -1);
        fclose(fd);
        break;
    case Pipe:
        int pipefd[2];
        if (pipe(pipefd) == -1)
        {
            std::cerr << PRINT_SHELL << PRINT_ERROR << ": Pipe error!" << std::endl;
            exit(1);
        }
        run_command(lhs, pipefd[1], -1, -1);
        close(pipefd[1]);
        run_command(rhs, -1, -1, pipefd[0]);
        close(pipefd[0]);
        break;
    case Redirect:
        fd = fopen(filename.c_str(), &open_mode);
        if (fd != nullptr)
        {
            run_command(lhs, -1, -1, fileno(fd));
            fclose(fd);
        }
        else
        {
            std::cerr << PRINT_SHELL << PRINT_ERROR << ": Redirect error! Failed to open " << filename << std::endl;
        }

        break;
    }

    print_prompt();
}

void format_input(std::string line) // this used to be parse
{
    // since the $ signifies variables, we will first find and replace them with their values
    size_t pos = 0;
    bool keepGoing = true;
    while (keepGoing)
    {
        // check to see if a $ exists
        size_t find = line.find('$', pos);
        if (find != std::string::npos)
        {
            // find the end of the current word
            size_t end = line.find(' ', find);

            // find the name of the var that we are substituing for
            std::string var = line.substr(find + 1, end - 1);

            // if no var exists, replace it with ""
            std::string replace = "";
            if (set.count(var))
            {
                replace = set[var];
            }
            line.replace(find, var.length() + 1, set[var]);

            // move pos forward
            pos = find;
        }
        else
        {
            // once all $ are substituted, stop the loop
            keepGoing = false;
        }
    }

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
    std::vector<std::string> splitCurrentLine = split_line(current_line);
    std::vector<std::string> splitAliasLine = findReplaceAlias(splitCurrentLine);

    std::vector<Token> tokens = lex(splitAliasLine);
    if (crash_debug)
    {
        for (size_t i = 0; i < tokens.size(); i++)
        {
            std::cout << PRINT_SHELL << PRINT_DEBUG << ": Token: " << i << " " << kwtype_as_string(tokens[i].type) << " Data: " << tokens[i].data << "\n";
        }
    }
    // ONLY PROCESS IF TOKENS SIZE IS NOT ZERO. ELSE SEGFAULTS WILL OCCUR.
    if (tokens.size() != 0)
    {
        process(tokens);
    }

    return;
}

int keyword_if(int argc, char **argv)
{
    // conditional if statements

    std::vector<std::string> args;

    // this stores the pairs of if, then statements
    std::vector<std::vector<std::string>> conditionals;

    // print out all of the arguments to test
    for (int i = 0; i < argc; i++)
    {
        // convert char** to string to make it easier to work with
        std::string tempStr = argv[i];
        args.push_back(tempStr);
        std::cout << args[i] << std::endl;
    }

    std::vector<std::string> tempVec;

    // parser, counters to keep track of if inside an if statement
    int ifCounter = 0;

    std::cout << "args --" << std::endl;
    for (unsigned int i = 0; i < args.size(); i++)
    {
        std::cout << args[i] << std::endl;
    }

    std::cout << std::endl
              << "parsing --" << std::endl;
    // for each word in the vector
    for (unsigned int i = 0; i < args.size(); i++)
    {

        if (args[i] == "if")
        {
            // increase ifCounter
            ifCounter++;
            if (ifCounter > 1)
            {
                tempVec.push_back(args[i]);
            }
        }
        else if (args[i] == "then")
        {
            // done counting variables
            if (ifCounter <= 1)
            {
                // in sub if, ignore keywords
                conditionals.push_back(tempVec);
                tempVec.clear();
            }
            else
            {
                // add to vector arguments
                tempVec.push_back(args[i]);
            }
        }
        else if (args[i] == "elseif")
        {
            // new statement
            if (ifCounter <= 1)
            {
                conditionals.push_back(tempVec);
                tempVec.clear();
                ifCounter++;
            }
            else
            {
                // end of sub if
                tempVec.push_back(args[i]);
            }
        }
        else if (args[i] == "else")
        {
            // last statement always happens
            if (ifCounter <= 1)
            {
                conditionals.push_back(tempVec);
                tempVec.clear();

                // add empty vector to check for when evaluating
                conditionals.push_back(tempVec);
            }
            else
            {
                // end of sub if
                tempVec.push_back(args[i]);
            }
        }
        else if (args[i] == "endif")
        {
            // end of the statement
            if (ifCounter <= 1)
            {
                conditionals.push_back(tempVec);
                tempVec.clear();
            }
            else
            {
                // end of sub if
                tempVec.push_back(args[i]);
            }
            ifCounter--;
        }
        else if (args[i] == "[")
        {
            // begining of elseif
            if (ifCounter <= 1)
            {
                ifCounter--;
            }
            else
            {
                tempVec.push_back(args[i]);
            }
        }
        else if (args[i] == "]")
        {
            if (ifCounter <= 1)
            {
                ifCounter--;
            }
            else
            {
                tempVec.push_back(args[i]);
            }
        }
        else
        {
            // not a keyword, so a command
            tempVec.push_back(args[i]);
        }
    }

    std::cout << std::endl
              << "conditionals --" << std::endl;
    for (unsigned long int j = 0; j < conditionals.size(); j++)
    {
        std::cout << j << ": ";
        for (unsigned int i = 0; i < conditionals[j].size(); i++)
        {
            std::cout << conditionals[j][i] << ",";
        }
        std::cout << std::endl;
    }

    // loop through each conditional with dictionary
    // return exit status

    return 0;
}
