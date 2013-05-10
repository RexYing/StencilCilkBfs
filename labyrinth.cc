#include "bfs.h"

#include <iostream>
#include <string>

using namespace std;

/*
 * return an array that represents the input
 * index 0: width; 1: height;
 * 2, 3: position of starting point;
 * 4, 5: position of destination
 * the reset width*height entries are the data
 */
int* process_input() {
    const int TASK_DESCRIPTION_SIZE = 6;
    int width, height;
    cin >> width >> height;
    int* array = malloc((width * height + TASK_DESCRIPTION_SIZE) * sizeof(int));
    array[0] = width;
    array[1] = height;
    cin >> array[2] >> array[3] >> array[4] >> array[5];
    string line;
    /*
     * read in "height" lines of strings, each string contains "width" characters.
     * in the array, 0 represents obstacle.
     * the aim is to find a path comprising of 1s.
     */
    int count = 0;
    for (int i = 0; i < height; i++) {
        cin >> line;
        for (string::iterator line_it = line.begin(); line_it < line.end(); line_it++) {
	    array[TASK_DESCRIPTION_SIZE + count] = *line_it; 
        }
    }
    for (int i = 0; i < width * height; i++) {

    }
}

int
main(int argc, char ** argv) {
    process_input();
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

