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
#define HISTORY_FILE_PATH (std::string(HOME) + "/crash_history.txt").c_str()

const std::string PROMPT_NEW = "$ ";
const std::string PROMPT_CNT = ">>> ";
#define PATH_MAX 1024

// shell fns
void parse(std::string line);
void print_prompt(void);

// history fns
int history_history_length(void);
void history_create_history_file(void);
void history_write_history_file(const std::string dir);

// aliases
extern std::unordered_map<std::string, std::string> aliases;

// builtin defs
int builtin_cd(int argc, char **argv);
int builtin_exit(int argc, char **argv);
int builtin_history(int argc, char **argv);
int builtin_alias(int argc, char **argv);
int builtin_unalias(int argc, char **argv);

//keywords
int keyword_if(int argc, char** argv);
