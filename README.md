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

### Meta characters
| Character  | Usage |
| ------ |:-------------:|
| \      | Continuation         |
| #      | Comment              |

### Interactive mode

```bash

# some example
this is the example command (todo: fill this)


# some example
this is the example command

```

### Batch Mode

To run a script in batch mode, there are 3 possible options.

#### Executable Script
- Give the script a path to CRASH by including `#!PATHTOCRASH` in the first line of the file. If in the test directory for example, use `#!./bin/main`.
- Make sure the file is executable by running `chmod +x FILENAME`
- Execute the script by running in a terminal `./SCRIPTNAME.EXTENSION`

#### Stdin Piping
Create a script and run `./bin/main < [script location]`

#### Script by Name
Create a script and run `./bin/main [script location]`