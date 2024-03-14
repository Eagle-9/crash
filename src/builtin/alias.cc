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
        std::cout << "not a known command. Did you mean alias -h or cd -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}

void alias_print(void)
{
    for(auto iter = aliases.begin(); iter!= aliases.end(); iter++)
    {
        std::cout << "alias " << iter->first << "='" << iter->second << "'" << std::endl; 
    }
}

void alias_parse(std::string line)
{
    // find the = sign first
    size_t find = line.find('=');
    std::string name = line.substr(0, find);
    std::string command = line.substr(find + 1);
    aliases.insert({name, command});
}

int builtin_alias(int argc, char **argv)
{
    // print out each alias if we just run 'alias' or with -p
    if(argc == 1 || (argc >= 2 && strcmp(argv[1],"-p") == 0))
    {
        alias_print();
    } 
    else if(argc >= 2 && (strcmp(argv[1],"-H") == 0 || strcmp(argv[1],"-h") == 0))
    {
        alias_help(argc, argv);
    }
    else {
        std::string line;
        for(int section = 1; section < argc; section++)
        {
            std::string i(argv[section]);
            line.append(i + ' ');
        }
        alias_parse(line);
    }
    return 0;
}