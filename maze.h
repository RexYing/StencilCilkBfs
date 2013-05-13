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
    std::pair<int, int> start_;
    std::pair<int, int> dest_;
    
  public:
    const int BLOCK = 0;
    const int PASS = 1;
    const int TASK_DESCRIPTION_SIZE = 6;
    int* ProcessInput();
    size_t get_data_size() { return data_size_; }
    std::pair<int, int> get_start() { return start_; }
    std::pair<int, int> get_dest() { return dest_; }
};

#ifdef NDEBUG
    #define DEBUG(M, ...)
#else
    #define DEBUG(M, ...) fprintf(stderr, "[DEBUG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#endif /* Rect Maze*/

