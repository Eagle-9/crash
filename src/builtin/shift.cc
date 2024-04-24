#include <crash.hh>

int builtin_shift(int argc, char **argv) {
    
    unsigned int shiftAmt = 0;

    //if no argument, default to shifting 1
    if (argc <= 1) {
        shiftAmt = 1;
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