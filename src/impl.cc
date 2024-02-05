/** 
 *
 *  impl.cc
 *      This file is for implementing "impl.hh"
 *
 */

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
    {"semicolon",THING}
};

//* function implementation

// this takes no arguments, as it uses 'line' as it's input.
// 'line' should be the parsed line from 'parse()'
std::string process()
{
    // todo: process parsed line
    std::string res;

    //get parsed line
    res = current_line;

    //split the current line by ' ' and set res to first word
    res = res.substr(0, res.find(' '));

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
