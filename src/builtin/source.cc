#include <cstdlib>
#include <crash.hh>
#define PRINT_SOURCE GREEN << "[SOURCE]" << RESET

int builtin_source(int argc, char **argv)
{
    if (argc == 2)
    {
        // open the file
        std::ifstream content;
        content.open(argv[1]);

        // make sure the file opened properly
        if (!content.is_open())
        {
            std::cerr << PRINT_SOURCE << PRINT_ERROR << ": Failed to open file: " << argv[1] << std::endl;
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
        return 0;
    }
    std::cerr << PRINT_SOURCE << PRINT_ERROR <<": NO FILE PROVIDED" << std::endl;
    return 1;
}