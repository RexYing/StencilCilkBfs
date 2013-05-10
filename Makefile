BASE := /opt/AMDAPP
INC := $(BASE)/include
LIB = $(BASE)/lib/x86_64 
CXX = g++ -w -O3 -I$(INC) -L$(LIB) -fpermissive
DCXX = g++ -w -g -I$(INC) -L$(LIB) -fpermissive
CC = gcc -w -O3 -I$(INC) -L$(LIB) -fpermissive

all: labyrinth

labyrinth: bfs.h labyrinth.cc
	$(CXX) labyrinth.cc -o labyrinth -lOpenCL
labyrinthDebug: labyrinth.h labyrinth.csc
	$(DCXX) labyrinth.cc -o labyrinthDebug -lOpenCL
clean:
	rm -f labyrinth labyrinthDebug
