#include <crash.hh>


int history_help_message(int argc, char ** argv)
{

    // simple help message
    std::string simpleHelp = "";

    // full help message
    std::string fullHelp = "";

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
        std::cout << "not a known command. Did you mean history -h or cd -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}


// this overload will print out the last n lines
// it is to be called from the normal cd_print_history
void history_print_history(int n)
{
    // get the number of lines in the file
    int totalLen = history_history_length();

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

int history_print_history(int argc, char **argv)
{
    // if we have an n for number of lines, we run the other function
    if (argc >= 3 && isdigit(argv[2][0]))
    {
        history_print_history(atoi(argv[2]));
    }
    else
    {
        // define and open the history file
        std::ifstream historyFile;
        historyFile.open(HISTORY_FILE_PATH);

        // check that the file is open
        if (historyFile.fail())
        {
            history_create_history_file();
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

int history_clear_history(int argc, char **argv)
{
    // delete the history file
    std::remove(HISTORY_FILE_PATH);

    // create a new history file
    history_create_history_file();
  
  return 0;
}

int history_nth_history(int argc, char **argv)
{
    int n = std::abs(atoi(argv[1])); //We need to get the absolute values of this, because the argument has a negative sign lol.
    // checks that everything is valid
    if (argc <= 2 && isdigit(argv[1][1]) && n < history_history_length())
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
            std::cout << "ERROR: Failed to change directory: " << dir << std::endl;
        }
        else
        {
            // log the history
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            history_write_history_file(std::string(cwd));
        }
    }
    else
    {
        std::cout << "INVALID NUMBER" << std::endl;
        return 1;
    }
    return 0;
}

int history_print_unique_history(int argc, char **argv)
{
    // create a vector to temporarily store the values
    std::vector<std::string> tempLine;

    // open up the history file
    std::ifstream historyFile;
    historyFile.open(HISTORY_FILE_PATH);

    // check that the history file is open
    if (historyFile.fail())
    {
        history_create_history_file();
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

int builtin_history(int argc, char **argv)
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
    std::unordered_map<std::string, int (*)(int argc, char ** argv)> history_table; // key = int, value is array of strings. all funcs must be formatted like 'void funcName(int argc, std::string* argv)'

    history_table["-h"] = history_help_message;         // displays a simple help message
    history_table["-H"] = history_help_message;         // displays a full help message
    history_table["-l"] = history_print_history;        // Display a history list
    history_table["-{n}"] = history_nth_history;        // Change current directory to nth element
    history_table["-c"] = history_clear_history;        // clean the directory history
    history_table["-s"] = history_print_unique_history; // suppress the directory history

    // make sure key is in table
    if (history_table.find(key) != history_table.end())
    {
        // access table
        return history_table[key](argc, argv);
    }
    else if (key[0] != '-')
    {
        if (chdir(key.c_str()) != 0)
        {
            std::cout << "ERROR: Invalid Directory: " << key << "\n";
            return 1;
        }
        else
        {
            char cwd[PATH_MAX];
            getcwd(cwd, sizeof(cwd));
            history_write_history_file(std::string(cwd));
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