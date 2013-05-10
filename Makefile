BASE := /opt/AMDAPP
INC := $(BASE)/include
LIB = $(BASE)/lib/x86_64 
CXX = g++ -w -O3 -I$(INC) -L$(LIB) -fpermissive
DCXX = g++ -w -g -I$(INC) -L$(LIB) -fpermissive
CC = gcc -w -O3 -I$(INC) -L$(LIB) -fpermissive

all: labyrinth

labyrinth: bfs.h bfs.cc labyrinth.h labyrinth.cpp
	$(CXX) bfs.cc labyrinth.cpp -o labyrinth -lOpenCL
labyrinthDebug: bfs.h bfs.csc labyrinth.h labyrinth.cpp
	$(DCXX) bfs.cc labyrinth.cpp -o labyrinthDebug -lOpenCL
clean:
	rm -f labyrinth labyrinthDebug
