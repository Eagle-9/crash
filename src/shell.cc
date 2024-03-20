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
/********************************************************************/
/*  Utility functions                                               */
/********************************************************************/

void print_prompt()
{
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    std::cout << "CRASH " << std::string(cwd) << " " << PROMPT_NEW;
}
std::string wildCardMatch(std::string wildCard) // Match wildcard to file in current directory
{
    glob_t globResult;
    int returnValue = glob(wildCard.c_str(), GLOB_TILDE, nullptr, &globResult);
    if (returnValue == 0)
    {
        return globResult.gl_pathv[0]; // Return first result
    }
    if (returnValue == GLOB_NOMATCH)
    {
        std::cout << "Err: no match found for wildcard" << std::endl;
        return wildCard;
    }
    else
    {
        std::cout << "Err: Failed glob wildcard search: " << returnValue << std::endl;
        return wildCard;
    }
    // Loop to get all matches if needed later, can be removed
    /*for (size_t i = 0; i < globResult.gl_pathc; i++) {
        std::cout << globResult.gl_pathv[i] << std::endl;
    }*/

    globfree(&globResult);
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

void run_external_fn(std::string *res, std::vector<char *> argv)
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
        std::string test_path = segment + "/" + argv[0];

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
        std::cout << "ERROR: Failed to find command: " << argv[0] << "\n";
    }
}

std::vector<Token> lex(std::vector<std::string> splitLineToParse)
{
    std::vector<Token> tokens;
    for (std::string entry : splitLineToParse)
    {
        Token newToken;

        // Check to see if token has a wildcard that needs to be converted
        size_t foundQuestionMark = entry.find('?');
        size_t foundAsterisk = entry.find('*');
        size_t foundLeftBracket = entry.find('[');
        size_t foundRightBracket = entry.find(']');
        // Convert entry to wildcard if needed
        if (foundQuestionMark != std::string::npos)
        { // We found a question mark
            entry = wildCardMatch(entry);
        }
        if (foundAsterisk != std::string::npos)
        { // We found an Asterisk
            entry = wildCardMatch(entry);
        }
        if (foundLeftBracket != std::string::npos && foundRightBracket != std::string::npos)
        { // We found a bracket pair
            if (foundLeftBracket < foundRightBracket)
            { // Make sure the right bracket is actually right of the left one
                entry = wildCardMatch(entry);
            }
        }

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
// TODO: Make this actually work. This right now is purely a stop gap that converts tokens back to a string so CRASH can still run.
void process(std::vector<Token> tokens)
{

    std::string res;
    std::vector<std::string> args;
    std::vector<std::vector<char>> holder;
    std::vector<char *> argv;

    // TODO: Remove this!
    // Temp workaround solution to let CRASH run with new parsing
    for (size_t i = 0; i < tokens.size(); i++)
    {
        args.emplace_back(tokens[i].data);
    }

    // get parsed line
    res = current_line;

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
        format_input(aliases.at(args[0]));
    }
    else
    {
        run_external_fn(&res, argv);
    } // end if
    std::cout << res << "\n";
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

    std::vector<Token> tokens = lex(split_line(current_line));

    for (size_t i = 0; i < tokens.size(); i++)
    {
        std::cout << "Token: " << i << " " << kwtype_as_string(tokens[i].type) << " Data: " << tokens[i].data << "\n";
    }
    process(tokens); // TODO: This is also part of the temp workaround that needs removed!
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
