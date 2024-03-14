#include <crash.hh>

int unalias_help(int argc, char **argv)
{
    // simple help message
    std::string simpleHelp = "unalias: unalias [-a] name [name ...]";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'alias'\n\nalias [-p] [name[=value] ... ]\n\nGeneral Use\n\nalias NAME=command\n\nArguments\n\n-h : Display simple help message\n-H : Display full help message\n-p : print all defined aliases";

    // differentiate between simple and complex help message
    if(argc == 1)
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
        std::cout << "not a known command. Did you mean alias -h or cd -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
}



int builtin_alias(int argc, char **argv)
{

    // help commands
    if (argc == 1 || argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        unalias_help(argc, argv);
    }

    // make an alias
    else
    {
        // let line be the final line. Simply append each of the argv's to it with spaces in the middle


    }
    return 0;
}