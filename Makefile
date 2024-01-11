CC 		= g++
INC_DIR = include
CFLAGS	= -Wall -g -fno-stack-protector -I${INC_DIR}
LDFLAGS =

SOURCES = src/main.cc src/classifier.cc

OBJ_OF = $(patsubst src/%.cc, build/%.o, $1 $2 $3)                                     
SRC_OF = $(patsubst build/%.o, src/%.cc, $1 $2 $3) 

# link
main: $(call OBJ_OF, $(SOURCES))
	mkdir -p bin && $(CC) $(CFLAGS) -o bin/main $(call OBJ_OF, $(SOURCES)) $(LDFLAGS)

# compile
$(call OBJ_OF, $(SOURCES)): $(SOURCES)
	mkdir -p $(@D) && $(CC) $(CFLAGS) -o $@ -c $(call SRC_OF, $@)

.PHONY: run
run: main
	cd bin && ./main

.PHONY: clean                                                                           
clean:
	rm -rf bin/ build/