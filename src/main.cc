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
            // todo: print what we're doing (echo) (do we need this?)
            // format_input(lines[i]);

            std::string currentLine = lines[i];
            std::string lineToInput;
            // Check if line needs to be processed in parts due to semicolons
            for (size_t i = 0; i < currentLine.length(); i++)
            {
                // Check if character is unquoted semicolon
                if (currentLine[i] == ';' && !isLocationInStringQuoted(currentLine, i))
                {                              // We found a semicolon, parse line and clear
                    format_input(lineToInput); // parse the line
                    lineToInput.clear();
                }
                else
                { // Current character is not a semicolon, so add it
                    lineToInput = lineToInput + currentLine[i];
                }
            }
            format_input(lineToInput); // This is needed because the above loop only runs input if there is a semicolon, we need to run the first part always
            lineToInput.clear();
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
        std::string lineToInput;
        // Check if line needs to be processed in parts due to semicolons
        for (size_t i = 0; i < line.length(); i++)
        {
            // Check if character is unquoted semicolon
            if (line[i] == ';' && !isLocationInStringQuoted(line, i))
            {                              // We found a semicolon, parse line and clear
                format_input(lineToInput); // parse the line
                lineToInput.clear();
            }
            else
            { // Current character is not a semicolon, so add it
                lineToInput = lineToInput + line[i];
            }
        }
        format_input(lineToInput); // This is needed because the above loop only runs input if there is a semicolon, we need to run the first part always

        // reset line
        line.clear();
    }
    return 0;
}
