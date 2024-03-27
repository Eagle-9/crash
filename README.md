# CRASH
*Crazy Random Awesome SHell*

CRASH is a C++ based shell that provides an interface to execute commands. Commands can be executed from either a user **Interactive Mode** or a script file in **Batch Mode**.

## Prerequisites
- make
- g++

## Installation

- Clone this repo
- Open the workspace in a terminal
- Use make commands to build

Make Targets:

| make target | description                        |
|-------------|------------------------------------|
| main        | Builds the main executable         |
| test        | Builds the test executable         |
| run         | Runs the main executable           |
| run-test    | Runs the test executable           |
| clean       | Clean build files from the project |

## Usage

The program can be used in two major ways:


### Interactive mode
Type external or internal commands directly into CRASH.

```bash
# Internal Command
CRASH $ history -l 5


# External Command
CRASH $ lsblk
```

### Batch Mode

To run a script in batch mode, there are 3 possible options.

#### Executable Script
- Give the script a path to CRASH by including `#!PATHTOCRASH` 
    - For example, use `#!./bin/main` if inside CRASH's directory, or if inside CRASH's test directory use `#!../bin/main`.
- Make the script executable by using `chmod +x SCRIPTNAME`
- Run the script `./SCRIPTNAME` or `SCRIPTNAME`


#### Stdin Piping
- Create a script and run `./bin/main < [script location]`

#### Script by Name
- Create a script and run `./bin/main [script location]`

## Character Usage

### Meta Characters
| Character  | Usage |
| ------ |:-------------:|
| \      | Continuation         |
| #      | Comment              |
| ;      | Split line           |
| >      | Store                |
| >>     | Append               |
| <      | Redirect             |
| 2>     | Store Error          |
| 2>>    | Append Error         |

### Meta Character Usage
- `\` Use backslack to continue a line
- `#` Use hashtags to add comments
- `;` Use semicolons to seperate lines to run multiple commands at once.
  - To run both the external `lsblk` and `lscpu` commands.
  - `CRASH $ lsblk ; lscpu`
- `>` Use a single greater than character to store the output from a command into a file.
  - To store the output of `lscpu` to a file called `output.txt`
  - `CRASH $ lscpu > output.txt`
- `>>` Use two greater than character to append the output from a command to a file.
  - To append the output of `lsblk` to a file called `output.txt` that already has existing content.
  - `CRASH $ lsblk >> output.txt`
- `|` Use a vertical bar to pipe the output of one program to another.
  - To pipe the output of `lscpu` to `sort` to have the output in alphabetical order.
  - `CRASH $ lscpu | sort`
- `<` Use a less than character to redirect the input of  program to a file.
  - To display the contents of a file called `test5.txt`
  - `CRASH $ cat < test5.txt`

### Wild Card Character Matching

Wild cards can be used to find matching files in the current working directory.

- Question marks (`?`) can be used to match any single character in a file name.
    - Example: `tes?.txt` can be matched to `test.txt`

- Asterisks (`*`) can be used to match any number of characters in a file name.
    - Example: `t*.txt` can be matched to `test.txt`

- Square brackets (`[...]`) can be used to match the characters inside the bracket.
    - Example: `test[0-9].txt` can be matched to `test5.txt`

Wildcards can be also be combined with meta characters for advanced functionality. 
- To store the names of all files in the current directory to `output.txt`
- `CRASH $ echo * > output.txt`