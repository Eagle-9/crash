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
    if(argc == 1)
    {
        // print out each help command
        for(auto iter = dict.begin(); iter != dict.end(); iter++)
        {
            if(iter->second.function_pointer == nullptr)
            {
                continue;
            }
            std::string line = iter->first;
            line.append(" -h");
            process(line);
        }
    }
    if (argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        help_help(argc, argv);
    }
    else
    {
        if(dict.count(argv[1]))
        {
            std::string command = argv[1];
            command += " -H";
            parse(command);
        }
        else
        {
            std::cout << argv[1] << " is not a valid command" << std::endl;
        }
    }

    return 0;
}