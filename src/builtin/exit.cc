#include <cstdlib>
#include <crash.hh>
#define PRINT_EXIT GREEN << "[EXIT]" << RESET

int builtin_exit(int argc, char **argv)
{
    if (argc >= 2)
    {
        if (strcmp(argv[1], "-h") == 0)
        {
            std::cout << "exit: exit" << std::endl;
            return 0;
        }
        else if (strcmp(argv[1], "-H") == 0)
        {
            std::cout << "CRASH MANUAL -- HOW TO USE 'exit'\n\nexit \n\nGeneral Use\n\nExit the current instance of CRASH.  There are no additional commands, arguments, or usage." << std::endl;
            return 0;
        }
    }
    if (crash_debug)
        std::cout << PRINT_EXIT << PRINT_DEBUG << ": EXIT" << std::endl;
    exit(0);
}