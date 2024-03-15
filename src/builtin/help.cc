#include <crash.hh>

int help_help(int argc, char **argv)
{
    // simple help message
    std::string simpleHelp = "help: help [NAME]";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'help'\n\nhelp [name]\n\nGeneral Use\n\nhelp NAME\n\nBy default, help outputs the basic help message for each available command.  help NAME, where NAME is a command will output the full help message for that command";

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
    if (argc == 1)
    {
        std::cout << std::endl;
        // print out each help command
        for (auto iter = dict.begin(); iter != dict.end(); iter++)
        {
            if (iter->second.function_pointer == nullptr)
            {
                continue;
            }

            char **full_cmd = new char *[2];
            full_cmd[0] = new char[15];
            full_cmd[1] = new char[3];
            strcpy(full_cmd[0], (iter->first).c_str());
            strcpy(full_cmd[1], "-h");

            iter->second.function_pointer(2, full_cmd);

            delete[] full_cmd[0];
            delete[] full_cmd[1];
            delete[] full_cmd;
        }
    }
    else if (argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        help_help(argc, argv);
    }
    else
    {
        if (dict.count(argv[1]))
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