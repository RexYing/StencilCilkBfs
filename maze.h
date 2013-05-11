#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>

#ifndef MAZE
#define MAZE

class RectMaze {
  
  private:
    int width_;
    int height_;
    int data_size_;
    
  public:
    const int BLOCK = 0;
    const int PASS = 1;
    const int TASK_DESCRIPTION_SIZE = 6;
    int* process_input();
    size_t get_data_size();
};

#ifdef NDEBUG
    #define DEBUG(M, ...)
#else
    #define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#endif /* Rect Maze*/

