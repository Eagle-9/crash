#include <crash.hh>

void alias_print(void)
{
    for(auto iter = aliases.begin(); iter!= aliases.end(); iter++)
    {
        std::cout << "alias " << iter->first << "='" << iter->second << "'" << std::endl; 
    }
}

int builtin_alias(int argc, char **argv)
{
    // print out each alias if we just run 'alias' or with -p
    if(argc == 1 || (argc >= 2 && strcmp(argv[1],"-p") == 0))
    {
        alias_print();
    } 
    else {
        
    }
    return 0;
}