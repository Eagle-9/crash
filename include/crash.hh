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
#include <fcntl.h>
#include <sys/stat.h>
#include <glob.h>
#define HOME getenv("HOME")
#define HISTORY_FILE_PATH (std::string(HOME) + "/crash_history.txt").c_str()

//Colors
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define PRINT_ERROR RED << "[ERROR]" << RESET
#define PRINT_WARN YELLOW << "[WARN]" << RESET
#define PRINT_DEBUG CYAN << "[DEBUG]" << RESET

const std::string PROMPT_NEW = "$ ";
const std::string PROMPT_CNT = ">>> ";
#define PATH_MAX 1024

// shell fns
bool isLocationInStringQuoted(std::string, size_t); // Quote detection is needed in main.cc
void format_input(std::string line);
void print_prompt(void);

// history fns
int history_history_length(void);
void history_create_history_file(void);
void history_write_history_file(const std::string dir);

// aliases and dict

enum TokenType
{
    Keyword,
    Argument,
    Internal, // Internal command
    External, // External command
    MetaChar
};

struct KeywordEntry
{
    TokenType keyword;
    int (*function_pointer)(int argc, char **argv);
};
extern std::unordered_map<std::string, KeywordEntry> dict;
extern std::unordered_map<std::string, std::string> aliases;

// set
extern std::unordered_map<std::string, std::string> set;
extern bool crash_debug;
extern bool crash_exit_on_err;

// builtin defs
int builtin_cd(int argc, char **argv);
int builtin_exit(int argc, char **argv);
int builtin_help(int argc, char **argv);
int builtin_history(int argc, char **argv);
int builtin_alias(int argc, char **argv);
int builtin_unalias(int argc, char **argv);
int builtin_set(int argc, char **argv);

//keywords
int keyword_if(int argc, char** argv);
