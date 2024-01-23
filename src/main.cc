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
