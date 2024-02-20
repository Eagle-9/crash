#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
// include implementation
#include <impl.hh>

int main(int argc, char** argv)
{
    if (argc == 2)
    {
        // open the file
        std::ifstream content;
        content.open(argv[1]);

        // make sure the file opened properly
        if (!content.is_open())
        {
            std::cerr << "Failed to open file: " << argv[1] << std::endl;
            return 1;
        }

        // to store all the lines
        std::vector<std::string> lines;
        // to store each line temporarily
        std::string line;
        // read all lines to 'lines'
        while (getline(content, line))
        {
          lines.push_back(line);
        }

        // close the file;
        // all lines are now in memory, in 'lines'
        content.close();

        for (size_t i = 0; i < lines.size(); i++)
        {
            // todo: print what we're doing (echo) (do we need this?)
            parse(lines[i]);
        }

        return 0;
    }

    // to store the line
    std::string line;
    // initial prompt
    std::string res;
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    res += cwd;
    res += " ";
    res += PROMPT_NEW;
    std::cout << res;

    // main loop
    while (std::getline(std::cin, line))
    {
	    line = strToLowerCase(line);

        // if exit, exit
        /* TODO: This probably needs to be implemented as it's own internal command
            At least that is how professor Kim had it.
        */
        if (line == "exit")
        {
            break;
        }

        // parse the line
        std::cout << parse(line);

        // reset line
        line.clear();
    }
    return 0;
}
