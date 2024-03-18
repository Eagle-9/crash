#include <crash.hh>

int set_help(int argc, char **argv)
{
    std::cout << "TODO: set help msg" << std::endl;
}

int set_parse(std::string line)
{
    // find the = sign first
    size_t find = line.find('=');

    // check for invalid string
    if (find == std::string::npos)
    {
        std::cout << "INVALID SET" << std::endl;
        return 1;
    }

    // the name will be from the beginning to the = sign
    std::string name = line.substr(0, find);

    // the command will be after the = to the end
    std::string command = line.substr(find + 1);

    // if the user puts in ' or " at the beg/end, remove it
    if (command[0] == '\'' || command[0] == '"')
    {
        command = command.substr(1);
    }

    // if the last line is a space, decriment it by 1
    while (command.back() == ' ')
    {
        command = command.substr(0, command.length() - 1);
    }

    size_t final_pos = command.length() - 1;
    if (command[final_pos] == '\'' || command[final_pos] == '"')
    {
        command = command.substr(0, final_pos);
    }

    // check if the variable already exists
    // if it exists, return an error
    if (set.count(name))
    {
        std::cout << "ERROR: Variable already exists.  Please use set -d to remove it before making a new one" << std::endl;

        return 1;
    }
    // insert the new variable
    set.insert({name, command});

    return 0;
}

void set_remove(int argc, char **argv)
{
    // let line be the final line. Simply append each of the argv's to it with spaces in the middle
    if (argc >= 2)
    {
        // get the name that was provided
        std::string name(argv[1]);
        if (set.count(name))
        {
            set.erase(name);
            std::cout << "Removed " << name << std::endl;
        }
        else
        {
            // error if alias is not found
            std::cout << "ERROR: " << name << " not found" << std::endl;
        }
    }
}

void set_print(void)
{
    // print out each set
    for (auto iter = set.begin(); iter != set.end(); iter++)
    {
        std::cout << iter->first << "='" << iter->second << "'" << std::endl;
    }
}
int builtin_set(int argc, char **argv)
{
    // print out each var if we just run 'var'
    if (argc == 1)
    {
        set_print();
    }

    // help commands
    else if (argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        set_help(argc, argv);
    }
    else if (argc >= 2 && strcmp(argv[1], "-a") == 0)
    {
        // delete all variables
        set.clear();
        std::cout << "All vars removed" << std::endl;
    }
    else if (argc >= 2 && strcmp(argv[1], "-d") == 0)
    {
        // delete all variables
        set_remove(argc, argv);
    }
    // make an var
    else
    {
        // let line be the final line. Simply append each of the argv's to it with spaces in the middle
        std::string line;
        for (int section = 1; section < argc; section++)
        {
            std::string i(argv[section]);
            line.append(i + ' ');
        }

        // pass it to our parse function
        set_parse(line);
    }
    return 0;
}