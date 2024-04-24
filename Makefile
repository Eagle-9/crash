# compiler
CC 		= g++
# include directory
INC_DIR = include
# compiler flags
CFLAGS	= -Wall -g -fno-stack-protector -I${INC_DIR}
# linker flags
LDFLAGS =

# source files
SOURCES = src/main.cc src/shell.cc src/builtin/cd.cc src/builtin/exit.cc src/builtin/history.cc src/builtin/alias.cc src/builtin/unalias.cc src/builtin/set.cc src/builtin/help.cc src/builtin/source.cc src/builtin/shift.cc

# file macros
OBJ_OF = $(patsubst src/%.cc,  build/%.o, $1 $2 $3)                                  
SRC_OF = $(patsubst build/%.o,  src/%.cc, $1 $2 $3)

# link
main: $(call OBJ_OF, $(SOURCES))
	mkdir -p bin && $(CC) $(CFLAGS) -o bin/main $(call OBJ_OF, $(SOURCES)) $(LDFLAGS)

# compile
$(call OBJ_OF, $(SOURCES)): $(SOURCES)
	mkdir -p $(@D) && $(CC) $(CFLAGS) -o $@ -c $(call SRC_OF, $@)

.PHONY: run
run: main
	cd bin && ./main

.PHONY: run-test
run-test: test
	cd bin && ./test

.PHONY: clean                                                                           
clean:
	rm -rf bin/ build/