#include <iostream>
#include <fstream>
#include <vector>
// include implementation
#include <impl.hh>

// test case location
const char FILE_PATH[] = "../test/content.txt";

int main()
{
    // open the test case file
    std::ifstream content;
    content.open(FILE_PATH);

    // make sure the file opened properly
    if (!content.is_open())
    {
        std::cerr << "Failed to open file: " << FILE_PATH << std::endl;
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

    // close the test case file;
    // all lines are now in memory, in 'lines'
    content.close();

    for (size_t i = 0; i < lines.size(); i++)
    {
        // todo: do some stuff
        std::cout << parse(lines[i]);
    }

    return 0;
}