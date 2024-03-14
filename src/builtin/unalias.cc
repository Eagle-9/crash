#include <crash.hh>

int unalias_help(int argc, char **argv)
{
    // simple help message
    std::string simpleHelp = "unalias: unalias [-a] name [name ...]";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'unalias'\n\nunalias [-a] name [name ...]\n\nGeneral Use\n\nunalias NAME\n\nArguments\n\n-h : Display simple help message\n-H : Display full help message\n-a : remove all alias definitions";

    // differentiate between simple and complex help message
    if (argc == 1)
    {
        std::cout << simpleHelp << std::endl; // simple help message
    }
    else if (strcmp(argv[1], "-h") == 0)
    {
        std::cout << simpleHelp << std::endl; // simple help message
    }
    else if (strcmp(argv[1], "-H") == 0)
    {
        std::cout << fullHelp << std::endl; // full help message
    }
    else
    {
        std::cout << "not a known command. Did you mean unalias -h or unalias -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}

void unalias_parse(int argc, char **argv)
{
    // let line be the final line. Simply append each of the argv's to it with spaces in the middle
    if (argc >= 2)
    {
        // get the name that was provided
        std::string name(argv[1]);
        if (aliases.count(name))
        {
            aliases.erase(name);
            std::cout << "Removed " << name << std::endl;
        }
        else
        {
            // error if alias is not found
            std::cout << name << " not found" << std::endl;
        }
    }
    else
    {
        unalias_help(argc, argv);
    }
}

int builtin_unalias(int argc, char **argv)
{

    // help commands
    if (argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        std::cout << "61" << std::endl;
        unalias_help(argc, argv);
    }
    else if(argc >= 2 && strcmp(argv[1], "-a") == 0)
    {
        // delete all aliases
        aliases.clear();
    }
    else
    {
        unalias_parse(argc, argv);
    }
    return 0;
}