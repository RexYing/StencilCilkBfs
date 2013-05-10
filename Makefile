BASE := /opt/AMDAPP
INC := $(BASE)/include
LIB = $(BASE)/lib/x86_64 
CXX = g++ -w -O3 -I$(INC) -L$(LIB) -fpermissive
DCXX = g++ -w -g -I$(INC) -L$(LIB) -fpermissive
CC = gcc -w -O3 -I$(INC) -L$(LIB) -fpermissive

all: maze

maze: bfs.h bfs.cc maze.h maze.cpp
	$(CXX) bfs.cc maze.cpp -o maze -lOpenCL
mazeDebug: bfs.h bfs.csc maze.h maze.cpp
	$(DCXX) bfs.cc maze.cpp -o mazeDebug -lOpenCL
clean:
	rm -f maze mazeDebug
