#include <crash.hh>

int builtin_shift(int argc, char **argv) {
    
    unsigned int shiftAmt = 0;

    //if no argument, default to shifting 1
    if (argc <= 1) {
        shiftAmt = 1;

    } else if (strcmp(argv[1], "-h") == 0) {
    
        //simple help message
        std::cout << "shift: shift {n} [-h] [-H]" << std::endl;
        return 0;

    } else if (strcmp(argv[1], "-H") == 0) {

        //full help message
        std::cout << "CRASH MANUAL -- HOW TO USE 'shift'\n\nshift {n} [-h] [-H]\nShift command takes one argument, a number. The positional parameters are shifted to the left by this number, N. The positional parameters from N+1 to $# are renamed to variable names from $1 to $# - N+1. make argv global\n{n} Number of arguments shifted\n[-h] Display simple help message\n[-H] Display full help message" << std::endl;
        return 0;

    } else {
        //if there is an argument, shift that many times
        shiftAmt = std::stoi(argv[1]);
    }

    //shift the arguments
    if (shiftAmt <= fileArgs.size()) {
        for (unsigned int i = 0; i < shiftAmt; i++) {
            fileArgs.erase(fileArgs.begin());
        }
    } else {
        return 1;
    }

    return 0;
}