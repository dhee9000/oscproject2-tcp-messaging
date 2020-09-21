CXX		  := g++
CXX_FLAGS := -Wall -Wextra -std=c++11 -ggdb -pthread

BIN		:= bin
SRC		:= src
INCLUDE	:= include
LIB		:= lib

LIBRARIES	:=
EXECUTABLE	:= main

TESTSCRIPT	:= test.sh

all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	./$(BIN)/$(EXECUTABLE) 0

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CXX) $(CXX_FLAGS) -I$(INCLUDE) -L$(LIB) $^ -o $@ $(LIBRARIES)

test:
	./$(TESTSCRIPT)

clean:
	-rm $(BIN)/*
