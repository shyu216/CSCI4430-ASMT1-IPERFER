## CSCI 4430 Advanced Makefile

# How to use this Makefile...
###################
###################
##               ##
##  $ make help  ##
##               ##
###################
###################

CXX = gcc
# TODO For C++ only.
CXXFLAGS = -std=c++14 -Wconversion -Wall -Werror -Wextra -pedantic 
# CXXFLAGS = -g -std=c++11 -pedantic

all: iPerfer

# TODO Modify source file name for your project.
# For C only.
# SOURCES = iPerfer.c
# For C++ only.
SOURCES = iPerfer.cpp
iPerfer: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o iPerfer

clean:
	rm -rf iPerfer *.dSYM

.PHONY: clean
