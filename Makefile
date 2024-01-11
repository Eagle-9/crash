# compiler
CC 		= g++
# include directory
INC_DIR = include
# compiler flags
CFLAGS	= -Wall -g -fno-stack-protector -I${INC_DIR}
# linker flags
LDFLAGS =

# source files
MAIN_SOURCES = src/main.cc
COMMON_SOURCES = src/impl.cc
TEST_SOURCES = src/test.cc

# file macros
OBJ_OF = $(patsubst src/%.cc,  build/%.o, $1 $2 $3)                                  
SRC_OF = $(patsubst build/%.o,  src/%.cc, $1 $2 $3)

# link
main: $(call OBJ_OF, $(MAIN_SOURCES) $(COMMON_SOURCES))
	mkdir -p bin && $(CC) $(CFLAGS) -o bin/main $(call OBJ_OF, $(MAIN_SOURCES)) $(call OBJ_OF, $(COMMON_SOURCES)) $(LDFLAGS)

test: $(call OBJ_OF, $(TEST_SOURCES) $(COMMON_SOURCES))
	mkdir -p bin && $(CC) $(CFLAGS) -o bin/test $(call OBJ_OF, $(TEST_SOURCES)) $(call OBJ_OF, $(COMMON_SOURCES)) $(LDFLAGS)

# compile
$(call OBJ_OF, $(MAIN_SOURCES)): $(MAIN_SOURCES)
	mkdir -p $(@D) && $(CC) $(CFLAGS) -o $@ -c $(call SRC_OF, $@)

$(call OBJ_OF, $(COMMON_SOURCES)): $(COMMON_SOURCES)
	mkdir -p $(@D) && $(CC) $(CFLAGS) -o $@ -c $(call SRC_OF, $@)

$(call OBJ_OF, $(TEST_SOURCES)): $(TEST_SOURCES)
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