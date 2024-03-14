#include <crash.hh>

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