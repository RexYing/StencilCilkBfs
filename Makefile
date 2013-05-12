BIN_PATH=bin
SRC_PATH=src
SOURCES=bfs.cc maze.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=maze

BASE := /opt/AMDAPP
INC := $(BASE)/include
LIB = $(BASE)/lib/x86_64 
CXX = g++ -w -O3 -I$(INC) -L$(LIB) -fpermissive
DCXX = g++ -w -g -I$(INC) -L$(LIB) -fpermissive
CC = gcc -w -O3 -I$(INC) -L$(LIB) -fpermissive

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) bfs.cc maze.cpp -o maze -lOpenCL
mazeDebug: bfs.h bfs.csc maze.h maze.cpp
	$(DCXX) bfs.cc maze.cpp -o mazeDebug -lOpenCL
clean:
	rm -f *.o maze mazeDebug $(BIN_PATH)/*
