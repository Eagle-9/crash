/** 
 *
 *  impl.hh
 *      This is the header for the "classifier",
 *      the implementation of project one.
 *
 *      This is separate from main to allow running
 *      both "main" and "test" in their own ways.
 *
 */

#pragma once
#include <string>

const std::string PROMPT_NEW = "$ ";
const std::string PROMPT_CNT = ">>> ";

// parse input (take each line and convert it to
// the form from "logical line"
std::string parse(std::string line);

// converts a string to lowercase
std::string strToLowerCase(std::string line);

// for testing: get the current state of the classifier
std::string _get_current();

//checks if a character is a metacharacter
bool checkMetacharacter(std::string inputString, size_t position);

//cd commands builtin

//int builtin_cd
std::string builtin_cd(int argc, std::string* argv);

//help message for cd passing h or H
int cd_help_message(std::string arg);
