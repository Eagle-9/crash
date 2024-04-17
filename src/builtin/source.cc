#include <cstdlib>
#include <crash.hh>
#define PRINT_SOURCE GREEN << "[SOURCE]" << RESET
int source_help_message(int argc, char **argv)
{
    // simple help message
    std::string simpleHelp = "source: source [-h] [-H] FILENAME";

    // full help message
    std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'source'\n\nsource [-h] [-H] FILENAME\n\nGeneral Use\n\nRead and execute commands from FILENAME\n\nArguments\n\n-h : Display simple help message\n-H : Display full help message";

    // differentiate between simple and complex help message
    if (strcmp(argv[1], "-h") == 0)
    {
        std::cout << simpleHelp << std::endl; // simple help message
    }
    else if (strcmp(argv[1], "-H") == 0)
    {
        std::cout << fullHelp << std::endl; // full help message
    }
    else
    {
        std::cerr << PRINT_SOURCE << PRINT_ERROR << ": Not a known command. Did you mean source -h or source -H ?" << std::endl; // not a known command
        return 1;
    }
    return 0;
};

int builtin_source(int argc, char **argv)
{

    if (argc == 2)
    {
        // help message
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-H") == 0)
        {
            return source_help_message(argc, argv);
        }
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
    std::cerr << PRINT_SOURCE << PRINT_ERROR << ": NO FILE PROVIDED" << std::endl;
    return 1;
}