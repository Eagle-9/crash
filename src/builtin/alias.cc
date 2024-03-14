#include <crash.hh>

int alias_help(int argc, char **argv)
{
    // simple help message
    std::string simpleHelp = "alias: alias [-p] [name[=value] ... ]";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'alias'\n\nalias [-p] [name[=value] ... ]\n\nGeneral Use\n\nalias NAME=command\n\nArguments\n\n-h : Display simple help message\n-H : Display full help message\n-p : print all defined aliases";

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
        std::cout << "not a known command. Did you mean alias -h or alias -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}

void alias_print(void)
{
    // print out each alias
    for (auto iter = aliases.begin(); iter != aliases.end(); iter++)
    {
        std::cout << "alias " << iter->first << "='" << iter->second << "'" << std::endl;
    }
}

int alias_parse(std::string line)
{
    // find the = sign first
    size_t find = line.find('=');

    // check for invalid string
    if (find == std::string::npos)
    {
        std::cout << "INVALID ALIAS" << std::endl;
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
        std::cout << "ERROR: Alias name already exists.  Please use unalias to remove it before making a new one" << std::endl;

        return 1;
    }

    // insert the new alias
    aliases.insert({name, command});

    return 0;
}

int builtin_alias(int argc, char **argv)
{
    // print out each alias if we just run 'alias' or with -p
    if (argc == 1 || (argc >= 2 && strcmp(argv[1], "-p") == 0))
    {
        alias_print();
    }

    // help commands
    else if (argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        alias_help(argc, argv);
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
        alias_parse(line);
    }
    return 0;
}