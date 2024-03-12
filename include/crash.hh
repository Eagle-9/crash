/**
 *
 *  crash.hh
 *      This is the main header for crash.
 *
 */

#pragma once
#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <sys/wait.h> // for wait()
#include <unistd.h>   // for exec()
#include <sys/stat.h>
#define HOME getenv("HOME")

const std::string PROMPT_NEW = "$ ";
const std::string PROMPT_CNT = ">>> ";
#define PATH_MAX 1024

// shell fns
void parse(std::string line);
void print_prompt();

// builtin defs
int builtin_cd(int argc, char **argv);
int builtin_exit(int argc, char **argv);