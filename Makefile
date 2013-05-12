BIN_PATH=bin
SRC_PATH=src
INC_DIR=include
SOURCES=bfs.cpp maze.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=maze

BASE := /opt/AMDAPP
INC := $(BASE)/include
LIB = $(BASE)/lib/x86_64 
CXX = g++ -w -O3 -I$(INC) -L$(LIB) -fpermissive
DCXX = g++ -w -g -I$(INC) -L$(LIB) -fpermissive
CC = gcc -w -O3 -I$(INC) -L$(LIB) -fpermissive

all: $(EXECUTABLE)

_DEPS=maze.h dfs.h
DEPS=$(patsubst %,$(INC_DIR)/%,$(_DEPS))

ODIR=obj
_OBJ=maze.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< -I$(INC_DIR)

$(EXECUTABLE): # $(OBJ)
	$(CXX) $(SOURCES) -o $(BIN_PATH)/$@ -lOpenCL
mazeDebug: bfs.h bfs.csc maze.h maze.cpp
	$(DCXX) bfs.cpp maze.cpp -o mazeDebug -lOpenCL

.PHONY: clean
clean: 
	rm -f *.o maze mazeDebug $(BIN_PATH)/* $(ODIR)/*
