#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
// include implementation
#include <crash.hh>
#define PRINT_MAIN GREEN << "[MAIN]" << RESET
bool isProcessingFile;
std::vector<std::string> fileArgs;

int main(int argc, char **argv)
{
    isProcessingFile = false;
    if (argc > 1)
    {
        // If there are more than 2 args, add them to a vector
        if (argc > 2)
        {
            for(int i = 1; i < argc; i++)
            {
                fileArgs.emplace_back(argv[i]);
                std::cout << "ADDING: " << argv[i] << std::endl;
            }
        }
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


        //storing whether $ is called
        bool isSubsitute = false;
        int start = 0;
        int end = 0;

        //temp nth argument index
        std::string tmpArgIndex = "";

        //for each line
        for (unsigned int l = 0; l < lines.size(); l++) {
            std::cout << lines[l] << std::endl;
            //for each character
            for (unsigned int k = 0; k < lines[l].length(); k++) {
                //if character is $
                if (lines[l][k] == '$') {
                    //need to check others
                    isSubsitute = true;
                    


                    /*
                    //store number given
                    int tmpNum = lines[l][k+1] - '0';
                    
                    //sub it with argument {n}
                    std::cout << "Num!!";
                    
                    //erase $ and {n}
                    lines[l].erase(k, 2);

                    //insert argument
                    std::string tmpArg = fileArgs[tmpNum];
                    lines[l].insert(k, tmpArg);

                    std::cout << tmpArg;
                    */
                } else if (isSubsitute == true) {
                    //check for kind
                    if (lines[l][k] == '#') {
                        //replace with argc
                        lines[l].erase(k-1, 2);
                        int numOfArgs = fileArgs.size();
                        std::string converted = std::to_string(numOfArgs);
                        lines[l].insert(k-1, converted);

                        //no longer sub
                        isSubsitute = false;
                    } else if (lines[l][k] == '*') {
                        //sub every argument
                        lines[l].erase(k-1, 2);

                        std::string allArgs = "";

                        //join the vector to make one giant string
                        for (unsigned int m = 0; m < fileArgs.size(); m++) {
                            allArgs.append(fileArgs[m]);
                            allArgs.append(" ");
                        }

                        //sub this in for $*
                        lines[l].insert(k-1, allArgs);

                        //no longer sub
                        isSubsitute = false;
                    } else if (isdigit(lines[l][k])) {
                        //is a number
                        std::cout << "I am a number" << std::endl;

                        //store k value where start
                        if (start == 0){
                            start = k-1;
                        }

                        //add to tmp
                        tmpArgIndex += lines[l][k];

                        //exception if this is last line
                        if (k+1 >= lines[l].length()) {
                            //erase start to end
                            end = k;
                            lines[l].erase(start, end-start+1);

                            //convert string to number
                            int iArgIndex = std::stoi(tmpArgIndex);

                            //use this number for array
                            std::string tmpArgument = fileArgs[iArgIndex];

                            //insert into line
                            lines[l].insert(start, tmpArgument);

                            //clear tmpArgIndex, start and end
                            tmpArgIndex = "";
                            start = 0;
                            end = 0;

                            //none of these !DONT CHECK
                            isSubsitute = false;
                        }

                    } else {

                        //if tmpArgIndex is not null, we need to process!
                        if (tmpArgIndex != "") {
                            //erase start to end
                            end = k;
                            lines[l].erase(start, end-start);

                            //convert string to number
                            int iArgIndex = std::stoi(tmpArgIndex);

                            //use this number for array
                            std::string tmpArgument = fileArgs[iArgIndex];

                            //insert into line
                            lines[l].insert(start, tmpArgument);

                        }

                        //clear tmpArgIndex, start and end
                        tmpArgIndex = "";
                        start = 0;
                        end = 0;

                        //none of these !DONT CHECK
                        isSubsitute = false;
                    }
                }
            }
        }

        std::cout << "CHANGED" << std::endl;

        for (unsigned int l = 0; l < lines.size(); l++) {
            std::cout << lines[l] << std::endl;
        }

        //subsitute any ${n} for the argument
        //              $* for every argument
        //              $# for number of arguments

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
