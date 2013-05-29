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
void InitDataStructure(const RectMaze &maze) {
    cl_int ret;
    inArray = clCreateBuffer(context,
                             CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                             maze.total_size() * sizeof(int),
                             NULL,//hostMem,
                             &ret);
    hostArray = (int *) clEnqueueMapBuffer(command_queue,
                                           inArray,
                                           CL_TRUE,
                                           CL_MAP_READ | CL_MAP_WRITE,
                                           0,
                                           maze.total_size() * sizeof(int),
                                           0,
                                           NULL,
                                           NULL,
                                           &ret);
    
    /*
     * fill in data to the hostArray
     */
    int* matadata = maze.MetadataArray();
    for (int i = 0; i < maze.kMetadataSize; i++) {
        hostArray[i] = matadata[i];
    }
    int index = maze.kMetadataSize;
    int* vertex_list = maze.vertex_list();
    for (int i = 0; i < maze.vertex_size(); i++) {
        hostArray[i + index] = vertex_list[i];
    }
    index += maze.vertex_size();
    int* edge_list = maze.edge_list();
    for (int i = 0; i < maze.edge_size(); i++) {
	hostArray[i + index] = edge_list[i];
    }

    clEnqueueUnmapMemObject(command_queue,
                                  inArray,
                                  hostArray,
                                  0,
                                  NULL,
                                  NULL);
 
    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;
}

/*
 * First argument: the log of the number of items.
 */
int
main(int argc, char ** argv) {
    RectMaze maze;
    maze.ProcessInput();

    num_stencil_items = 2 << atoi(argv[1]);
    initStencil(argc, num_stencil_items, maze, argv);
    InitDataStructure(maze);

    runStencil();

    printResult();
    cleanStencil();

    return 0;
}


