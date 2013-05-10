#include "bfs.h"
#include "labyrinth.h"

#include <iostream>
#include <string>

using namespace std;

int
main(int argc, char ** argv) {
    RectLabyrinth labyrinth;
    labyrinth.process_input();
    initStencil(argc, argv);

    // initialize own data structures here
    cl_int ret;
    inArray = clCreateBuffer(context,
                             CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                             2 * inSize * sizeof(int),
                             NULL,//hostMem,
                             &ret);

    hostArray = (int *) clEnqueueMapBuffer(command_queue,
                                           inArray,
                                           CL_TRUE,
                                           CL_MAP_READ | CL_MAP_WRITE,
                                           0,
                                           2 * inSize * sizeof(int),
                                           0,
                                           NULL,
                                           NULL,
                                           &ret);
    for ( int i = 0; i < inSize; i++) {
        int tmp = get_random() % (10 * inSize);
        hostArray[i] = tmp;
        hostArray[i +inSize] = tmp;
    }
    clEnqueueUnmapMemObject(command_queue,
                                  inArray,
                                  hostArray,
                                  0,
                                  NULL,
                                  NULL);
 
    runStencil();

    if (argc > 3)
        printResult();
    cleanStencil();

    return 0;
}

