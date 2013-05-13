BIN_PATH=bin
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

$(ODIR)/%.o: %.c $(DEPS)
	$(CXX) -c -o $@ $< -I$(INC_DIR)

$(EXECUTABLE): 
	$(CXX) $(SOURCES) -o $(BIN_PATH)/$@ -lOpenCL
mazeDebug: bfs.h bfs.csc maze.h maze.cpp
	$(DCXX) bfs.cpp maze.cpp -o mazeDebug -lOpenCL

.PHONY: clean
clean: 
	rm -f *.o maze mazeDebug $(BIN_PATH)/* 
