#include <cstdlib>
#include <crash.hh>

int builtin_exit(int argc, char **argv)
{
    if(argc >= 2 && (strcmp(argv[1], "-H") == 0 || strcmp(argv[1], "-h") == 0))
    {
        std::cout << "exit: exit" << std::endl;
        return 0;
    }
    exit(0);
}