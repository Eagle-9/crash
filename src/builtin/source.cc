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

    if (argc > 1)
    {
        // help message
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-H") == 0)
        {
            return source_help_message(argc, argv);
        }

        // If there are more than 2 args, add them to a vector
        if (argc > 2)
        {
            for (int i = 1; i < argc; i++)
            {
                fileArgs.emplace_back(argv[i]);
            }
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

        // storing whether $ is called
        bool isSubsitute = false;
        int start = 0;
        int end = 0;

        // temp nth argument index
        std::string tmpArgIndex = "";

        // close the file;
        // all lines are now in memory, in 'lines'
        content.close();
        isProcessingFile = true;
        for (size_t i = 0; i < lines.size(); i++)
        {
            // subsitute the current line
            for (unsigned int k = 0; k < lines[i].length(); k++)
            {
                // if character is $
                if (lines[i][k] == '$')
                {
                    // need to check others
                    isSubsitute = true;
                }
                else if (isSubsitute == true)
                {
                    // check for kind
                    if (lines[i][k] == '#')
                    {
                        // replace with argc
                        lines[i].erase(k - 1, 2);
                        int numOfArgs = fileArgs.size();
                        std::string converted = std::to_string(numOfArgs);
                        lines[i].insert(k - 1, converted);

                        // no longer sub
                        isSubsitute = false;
                    }
                    else if (lines[i][k] == '*')
                    {
                        // sub every argument
                        lines[i].erase(k - 1, 2);

                        std::string allArgs = "";

                        // join the vector to make one giant string
                        for (unsigned int m = 0; m < fileArgs.size(); m++)
                        {
                            allArgs.append(fileArgs[m]);
                            allArgs.append(" ");
                        }

                        // sub this in for $*
                        lines[i].insert(k - 1, allArgs);

                        // no longer sub
                        isSubsitute = false;
                    }
                    else if (isdigit(lines[i][k]))
                    {
                        // is a number

                        // store k value where start
                        if (start == 0)
                        {
                            start = k - 1;
                        }

                        // add to tmp
                        tmpArgIndex += lines[i][k];

                        // exception if this is last line
                        if (k + 1 >= lines[i].length())
                        {
                            // erase start to end
                            end = k;
                            lines[i].erase(start, end - start + 1);

                            // convert string to number
                            int iArgIndex = std::stoi(tmpArgIndex);

                            // use this number for array
                            std::string tmpArgument = fileArgs[iArgIndex];

                            // insert into line
                            lines[i].insert(start, tmpArgument);

                            // clear tmpArgIndex, start and end
                            tmpArgIndex = "";
                            start = 0;
                            end = 0;

                            // none of these !DONT CHECK
                            isSubsitute = false;
                        }
                    }
                    else
                    {

                        // if tmpArgIndex is not null, we need to process!
                        if (tmpArgIndex != "")
                        {
                            // erase start to end
                            end = k;
                            lines[i].erase(start, end - start);

                            // convert string to number
                            int iArgIndex = std::stoi(tmpArgIndex);

                            // use this number for array
                            std::string tmpArgument = fileArgs[iArgIndex];

                            // insert into line
                            lines[i].insert(start, tmpArgument);
                        }

                        // clear tmpArgIndex, start and end
                        tmpArgIndex = "";
                        start = 0;
                        end = 0;

                        // none of these !DONT CHECK
                        isSubsitute = false;
                    }
                }
            }

            // format the input
            format_input(lines[i]);
        }
        isProcessingFile = false;
        return 0;
    }
    std::cerr << PRINT_SOURCE << PRINT_ERROR << ": NO FILE PROVIDED" << std::endl;
    return 1;
}