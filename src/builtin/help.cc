#include <crash.hh>

int help_help(int argc, char **argv)
{
    // simple help message
    std::string simpleHelp = "help: help [NAME]";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'help'\n\nhelp [name]\n\nGeneral Use\n\nhelp NAME\n\nBy default, help outputs the basic help message for each available command.  help NAME, where NAME is a command will output the full help message for that command";

    // differentiate between simple and complex help message
    if (argc == 1 || strcmp(argv[1], "-h") == 0)
    {
        std::cout << simpleHelp << std::endl; // simple help message
    }
    else if (strcmp(argv[1], "-H") == 0)
    {
        std::cout << fullHelp << std::endl; // full help message
    }
    else
    {
        std::cout << "not a known flag. Did you mean help -h or help -H ?" << std::endl; // not a known command
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

            // to avoid using the heap, we will create an array of pointers
            char* full_cmd[2];

            // we will then make two more arrays that are the needed size
            char cmdBuffer[iter->first.length() + 1];
            char flagBuffer[3];

            // copy those strings into our buffers
            strcpy(cmdBuffer, (iter->first).c_str());
            strcpy(flagBuffer, "-h");

            // set the pointers to point to the buffers
            full_cmd[0] = cmdBuffer;
            full_cmd[1] = flagBuffer;

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