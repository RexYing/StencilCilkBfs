#include "bfs.h"
#include "maze.h"

#include <iostream>
#include <string>

using namespace std;

/* 
 * initialize own data structures for host.
 * maze: maze object
 * inputArray: 1-D array that describes the maze task
 */
void initialize_data_structure(const RectMaze &maze, const int* inputArray) {
    cl_int ret;
    inArray = clCreateBuffer(context,
                             CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                             maze.get_data_size() * sizeof(int),
                             NULL,//hostMem,
                             &ret);
    hostArray = (int *) clEnqueueMapBuffer(command_queue,
                                           inArray,
                                           CL_TRUE,
                                           CL_MAP_READ | CL_MAP_WRITE,
                                           0,
                                           maze.get_data_size() * sizeof(int),
                                           0,
                                           NULL,
                                           NULL,
                                           &ret);
    for (int i = 0; i < maze.get_data_size(); i++) {
        hostArray[i] = inputArray[i];
        cout << hostArray[i] << " ";
    }
    cout << endl;
    clEnqueueUnmapMemObject(command_queue,
                                  inArray,
                                  hostArray,
                                  0,
                                  NULL,
                                  NULL);
 
}

int
main(int argc, char ** argv) {
    RectMaze maze;
    int* input_data = maze.process_input();
    initStencil(argc, argv);
    initialize_data_structure(maze, input_data);

    runStencil();

    if (argc > 3)
        printResult();
    cleanStencil();

    return 0;
}


