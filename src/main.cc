#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
// include implementation
#include <crash.hh>
#define PRINT_MAIN GREEN << "[MAIN]" << RESET
bool isProcessingFile;

int main(int argc, char **argv)
{
    isProcessingFile = false;
    if (argc == 2)
    {
        // open the file
        std::ifstream content;
        content.open(argv[1]);

        // make sure the file opened properly
        if (!content.is_open())
        {
            std::cerr << PRINT_MAIN << PRINT_ERROR << ": Failed to open file: " << argv[1] << std::endl;
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
        isProcessingFile = true;
        for (size_t i = 0; i < lines.size(); i++)
        {
            format_input(lines[i]);
        }
        isProcessingFile = false;
        return 0;
    }

    // to store the line
    std::string line;
    // initial prompt
    print_prompt();

    // main loop
    while (std::getline(std::cin, line))
    {
        format_input(line);
        line.clear(); // reset line
    }
    return 0;
}
