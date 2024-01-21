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

const std::string PROMPT_NEW = "\nBETA $ ";
const std::string PROMPT_CNT = ">>> ";

// parse input (take each line and convert it to
// the form from "logical line"
std::string parse(std::string line);

// for testing: get the current state of the classifier
std::string _get_current();