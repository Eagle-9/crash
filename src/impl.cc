/** 
 *
 *  impl.cc
 *      This file is for implementing "impl.hh"
 *
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <impl.hh>
#include <vector>
#define PERSON "person"
#define PLACE "place"
#define THING "thing"
#define INTERNAL "internal"
#define KEYWORD "keyword"
//* state info

// if we're currently in a continuation
bool is_continuation = false;
// the current line
std::string current_line;

//* const info

// classification table
// the three classifications are "person", "place", "thing"
std::unordered_map<std::string, std::string> dict =
{
    {"mason",PERSON},
    {"indianapolis",PLACE},
    {"compiler",THING},
    {"i",PERSON},
    {"paris",PLACE},
    {"duck",THING},
    {"john",PERSON},
    {"cedar lake",PLACE},
    {"dinosaur",THING},
    {"nick",PERSON},
    {"boston",PLACE},
    {"computer",THING},
    {"dylan",PERSON},
    {"london",PLACE},
    {"lego",THING},
    {"jeff",PERSON},
    {"beijing",PLACE},
    {"bottle",THING},
    {"kim",PERSON},
    {"tokyo",PLACE},
    {"camera",THING},
    {"washington",PERSON},
    {"delhi",PLACE},
    {"lines",THING},
    {"andy",PERSON},
    {"orlando",PLACE},
    {"calculator",THING},
    {"linus",PERSON},
    {"gary",PLACE},
    {"semicolon",THING},
    {"alias",INTERNAL},
    {"bg",INTERNAL},
    {"cd",INTERNAL},
    {"eval",INTERNAL},
    {"exec",INTERNAL},
    {"exit",INTERNAL},
    {"export",INTERNAL},
    {"fc",INTERNAL},
    {"fg",INTERNAL},
    {"help",INTERNAL},
    {"history",INTERNAL},
    {"jobs",INTERNAL},
    {"let",INTERNAL},
    {"local",INTERNAL},
    {"logout",INTERNAL},
    {"read",INTERNAL},
    {"set",INTERNAL},
    {"shift",INTERNAL},
    {"shopt",INTERNAL},
    {"source",INTERNAL},
    {"unalias",INTERNAL},
    {"break",KEYWORD},
    {"continue",KEYWORD},
    {"do",KEYWORD},
    {"else",KEYWORD},
    {"elseif",KEYWORD},
    {"end",KEYWORD},
    {"endif",KEYWORD},
    {"for",KEYWORD},
    {"function",KEYWORD},
    {"if",KEYWORD},
    {"in",KEYWORD},
    {"return",KEYWORD},
    {"then",KEYWORD},
    {"until",KEYWORD},
    {"while",KEYWORD}
};

//* function implementation

// this takes no arguments, as it uses 'line' as it's input.
// 'line' should be the parsed line from 'parse()'
std::string process()
{
    std::string res;
    std::vector<std::string> args;

    //get parsed line
    res = current_line;

    //Go through every character in the line, split them into args
    std::string tempArg;
    for(size_t i=0; i < res.length(); i++){
        if (res[i] != ' '){ //If the current space isn't blank, add to tempArg
            tempArg = tempArg + res[i];
        }else{ //We hit a blank space, so we split the line. Add current arg to args, reset temp arg.
            args.emplace_back(tempArg);
            tempArg = "";
        }
    }
    //The above loop only adds an argument if there is a space, so we need this to get the inital arg.
    if (res.length() != 0){
        args.emplace_back(tempArg);
    }

    for (size_t i = 0; i < args.size(); i++){
        std::cout << "ARG[" << i << "]: " << args[i] << std::endl;;
    }

    //if the map returns a key
    if (dict.count(res)) {
        
	//get class from dictionary
        std::string lineClassName = dict.at(res);
    
	//append class to line
    	res = res + " " + lineClassName;

    } else {
    
	//not in dictionary
	res = res + " external";
    
    }//end if

    // add prompt to end of response
    res += "\n";
    res += PROMPT_NEW;
    
    // clear current line + return
    current_line.clear();
    return res;
}

// commented in header
std::string parse(std::string line)
{
    // if there's a blank line
    if (line.length() == 0)
    {
        return PROMPT_NEW;
    }

    // Dylan: Remove extra whitespace from the line.
    std::string tempLine;
    bool encounteredFirstChar = false;
    for (size_t i = 0; i < line.length(); i++){
        if (line[i] == ' ' || line[i] == '\t'){ //Check and see if we have found a space or tab.
            //First check to make sure we are not at the end of the line and we have had a character
            if (i+1 < line.length() && encounteredFirstChar){
                //If the next character is not a space/tab, we can add a space.
                if (line[i+1] != ' ' && line[i+1] != '\t'){
                    tempLine += ' ';
                }
            }
        }else{ //The character was not a space or tab, so just add it
            tempLine += line[i];
            encounteredFirstChar = true;
        }
    }
    line = tempLine;

    // to store the comment start location
    size_t comment_start = std::string::npos;
    // for char in line, search for a comment start location
    for (size_t i = 0; i < line.length(); i++)
    {
        if (
            line[i] == '#' && // we have a pound and one of the following
            (
                i == 0  || // the pound is the first character; full-line comment
                (line[i - 1] != '\'' && line[i - 1] != '"') || // the pound doesn't start with a quote
                (line[i + 1] != '\'' && line[i + 1] != '"') // the pound doesn't end with a quote
            )
        ) {
            // we found a comment start location
            comment_start = i;
            break;
        }
    }

    // if we found a comment start location
    if (comment_start != std::string::npos)
    {
        // if it's a full line comment, skip the line
        if (comment_start == 0)
        {
            if (!is_continuation)
            {
                return PROMPT_NEW;                
            }
            line = "";
        } else {
            line = line.substr(0, comment_start);
        }
    }

    // if there's a continuation
    if ( line != "" && line[line.length() - 1] == '\\')
    {
        line.pop_back();
        is_continuation = true;
    } else {
        is_continuation = false;
    }


    // append parsed to current_line
    current_line += line;
    // is this a continuation?
    if (is_continuation)
    {
        return PROMPT_CNT;
    }
    // not a continuation, as we would've returned
    return process();
}

std::string strToLowerCase(std::string line) {
//this turns a string to lower case

  for (int i = 0; (unsigned)i < line.size(); i++) {
    //convert character to lower case
    line[i] = tolower(line[i]);
  }//end for

  return line;

}//end strToLowerCase

// commented in header
std::string _get_current()
{
    return current_line;
}

//CD COMMANDS

int cd_help_message(std::string arg) {

  //simple help message
  std::string simpleHelp = "To change directory, input 'cd DIR' where DIR is the desired directory's address";
          
  //full help message
  std::string fullHelp = "CRASH MANUAL -- HOW TO USE 'cd'\n\ncd [-h] [-H] [-l [{n}]] [-{n}] [-c] [-s] (DIR)\n\nGeneral Use\n\nChange the current directory to DIR. The default DIR is the current directory so that it is identical to the pwd command in typical Linux shells\n\nArguments\n\n-h : Display simple help message\n-H : Display full help message\n-l [{n}] : Display the history list of the current directories with serial numbers. With the optional N it displays only the last N entries\n-{n} : Change the current directory to the n-th entry in the history list\n-c : Clean the directory history\n-s : Suppress the directory history by eliminating duplicated directories. The order of the history must be maintained. The latest entry must be kept for same directories";
                      
  //differentiate between simple and complex help message
   if (arg == "h") {
    std::cout << simpleHelp << std::endl; //simple help message
  } else if (arg == "H") {
    std::cout << fullHelp << std::endl; //full help message
  } else {
    std::cout << "not a known command. Did you mean cd -h or cd -H ?" << std::endl; //not a known command
  }

  return 0;
}
