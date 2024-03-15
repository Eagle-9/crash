#include <crash.hh>

int help_help(int argc, char **argv)
{
    // simple help message
    std::string simpleHelp = "unalias: unalias [-a] name [name ...]";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'help'\n\nunalias [-a] name [name ...]\n\nGeneral Use\n\nunalias NAME\n\nArguments\n\n-h : Display simple help message\n-H : Display full help message\n-a : remove all alias definitions";

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
        std::cout << "not a known command. Did you mean help -h or help -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}

int builtin_help(int argc, char **argv)
{


    return 0;
}