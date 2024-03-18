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

    // check if the alias already exists
    // if it exists, return an error
    if (aliases.count(name))
    {
        std::cout << "ERROR: Variable already exists.  Please use set -d to remove it before making a new one" << std::endl;

        return 1;
    }
    // insert the new alias
    aliases.insert({name, command});

    return 0;
}

void set_print(void)
{
    // print out each alias
    for (auto iter = aliases.begin(); iter != aliases.end(); iter++)
    {
        std::cout << iter->first << "='" << iter->second << "'" << std::endl;
    }
}
int builtin_set(int argc, char **argv)
{
    // print out each alias if we just run 'alias' or with -p
    if (argc == 1 || (argc >= 2 && strcmp(argv[1], "-p") == 0))
    {
        set_print();
    }

    // help commands
    else if (argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        set_help(argc, argv);
    }

    // make an alias
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