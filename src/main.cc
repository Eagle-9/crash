#include <iostream>
// include implementation
#include <impl.hh>

int main()
{
    // to store the line
    std::string line;
    // initial prompt
    std::cout << PROMPT_NEW;

    // main loop
    for (;;)
    {
        // get the line from user
        std::getline(std::cin, line);
	
	    line = strToLowerCase(line);

        // if exit, exit
        /* TODO: This probably needs to be implemented as it's own internal command
            At least that is how professor Kim had it.
        */
        if (line == "exit")
        {
            break;
        }


	//TEST FOR CD REMOVE
	if (line == "cd") {
	  //test
	  std::string testLine[2] = {"cd","-h"};

	  builtin_cd(2, testLine);
	}//end line

        // parse the line
        std::cout << parse(line);

        // reset line
        line.clear();
    }
    return 0;
}
