#ifndef _CILK_BFS_H_
#define _CILK_BFS_H_

#include <CL/opencl.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <linux/limits.h>
#endif

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
#define _aligned_malloc __mingw_aligned_malloc
#define _aligned_free  __mingw_aligned_free
#endif // __MINGW32__  and __MINGW64_VERSION_MAJOR


#ifndef _WIN32
#if defined(__INTEL_COMPILER)
#pragma warning(disable : 1125)
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include "maze.h"

#define MAX_SOURCE_SIZE (0x100000)
#define STENCIL_ITEMS    (0x20000)
#define STENCIL_SIZE(X) (9 * (sizeof(int) * (X)))

int m_w = 123;
int m_z = 234;
inline int get_random() {
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return abs((m_z << 16) + m_w);
}

cl_command_queue command_queue;

cl_context context;

cl_kernel kernel;

cl_kernel cleanKernel;

cl_program program;

cl_platform_id platform_id;

cl_device_id device_id;

cl_uint ret_num_devices;

cl_uint ret_num_platforms;

/* of max number of available threads */
int num_stencil_items;

int stencilSize;
int input_array_size;

int start_vertex;
int dest_vertex;

cl_mem stencilMem;

int * hostMem;

cl_mem inArray;

int * hostArray;

size_t global_size[1];

size_t local_size[1];

// CilkStencil type
typedef struct {
    cl_mem taskTail;
    cl_mem freeTail;
    cl_mem freeList;
    cl_mem taskIds;
    cl_mem syncEpochs;
    cl_mem parentIds;
    cl_mem val0s; //row number of starting position
    cl_mem val1s; //column number of starting position
    cl_mem val2s; //size | low2
    cl_mem val3s; //---  | high2
    cl_mem val4s; //---- | lowdest
} CilkStencil;

/* all tasks */
typedef struct {
    volatile int * taskTail;
    volatile int * freeTail;
    int * freeList;
    int * taskIds;
    int * syncEpochs;
    int * parentIds;
    int * val0s; //row number of starting position
    int * val1s; //column number of starting position
    int * val2s; //size | low2
    int * val3s; //---  | high2
    int * val4s; //---- | lowdest
} HostStencil;

HostStencil hostInput;

CilkStencil cilkInput;

void printQueue(HostStencil queue) {
    int tid = 0;
    printf("taskTail[%d] = %d\n", tid, queue.taskTail[0]);
    printf("freeTail[%d] = %d\n", tid, queue.freeTail[0]);
    printf("freeList[%d] = %d\n", tid, queue.freeList[tid]);
    printf("taskIds[%d] = %d\n", tid, queue.taskIds[tid]);
    printf("syncEpochs[%d] = %d\n", tid, queue.syncEpochs[tid]);
    printf("parentIds[%d] = %d\n", tid, queue.parentIds[tid]);
    printf("val0s[%d] = %d\n", tid, queue.val0s[tid]);
    printf("val1s[%d] = %d\n", tid, queue.val1s[tid]);
    if (tid == 0) {
        printf("taskTail= %p\n", queue.taskTail);
        printf("freeTail %p\n", queue.freeTail);
        printf("freeList = %p\n", queue.freeList);
        printf("taskIds = %p\n", queue.taskIds);
        printf("syncEpochs = %p\n", queue.syncEpochs);
        printf("parentIds = %p\n", queue.parentIds);
        printf("val0s = %p\n", queue.val0s);
        printf("val1s = %p\n", queue.val1s);
    }
}
void printDevQueue(CilkStencil queue) {
#if 0
    int tid = 0;
    printf("taskTail[%d] = %x\n", tid, ((int *) queue.taskTail)[0]);
    printf("freeTail[%d] = %x\n", tid, ((int *) queue.freeTail)[0]);
    printf("freeList[%d] = %x\n", tid, ((int *) qqueue.freeList)[tid]);
    printf("taskIds[%d] = %x\n", tid, ((int *) queue.taskIds)[tid]);
    printf("syncEpochs[%d] = %x\n", tid, ((int *) queue.syncEpochs)[tid]);
    printf("parentIds[%d] = %x\n", tid, ((int *) queue.parentIds)[tid]);
    printf("val0s[%d] = %x\n", tid, ((int *) queue.val0s)[tid]);
    printf("val1s[%d] = %x\n", tid, ((int *) queue.val1s)[tid]);
#endif
        printf("taskTail= %p\n", queue.taskTail);
        printf("freeTail %p\n", queue.freeTail);
        printf("freeList = %p\n", queue.freeList);
        printf("taskIds = %p\n", queue.taskIds);
        printf("syncEpochs = %p\n", queue.syncEpochs);
        printf("parentIds = %p\n", queue.parentIds);
        printf("val0s = %p\n", queue.val0s);
        printf("val1s = %p\n", queue.val1s);
}
inline void setCilkStencil() {
    int s;
#if 0
    int * a = (int *) stencilMem;
    cilkInput.freeList = (cl_mem) a;
    cilkInput.taskIds = (cl_mem) (a += s);
    cilkInput.syncEpochs = (cl_mem) (a += s);
    cilkInput.parentIds = (cl_mem) (a += s);
    cilkInput.val0s = (cl_mem) (a += s);
    cilkInput.val1s = (cl_mem) (a += s);
#endif
    cilkInput.taskTail = clCreateBuffer(context,
                                CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                                4,
                                NULL,//hostInput.taskTail,
                                &s);

    cilkInput.freeTail = clCreateBuffer(context,
                                CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                                4,
                                NULL,//hostInput.freeTail,
                                &s);

}

/*
 * set host data (for all threads)
 */
inline void setHostStencil() {
    int * a = hostMem;
    int s = num_stencil_items;
    hostInput.freeList = a;
    hostInput.taskIds = (a += s);
    hostInput.syncEpochs = (a += s);
    hostInput.parentIds = (a += s);
    hostInput.val0s = (a += s);
    hostInput.val0s[0] = start_vertex;

    hostInput.val1s = (a += s);
    hostInput.val1s[0] = dest_vertex;

    hostInput.val2s = (a += s);

    hostInput.val3s = (a += s);
    hostInput.val3s[0] = 0;
    hostInput.val4s = (a += s);
    hostInput.val4s[0] = 0;
    hostInput.taskTail[0] = 1;
    hostInput.freeTail[0] = 0;
    for (int i = 0; i < s; i++) {
        hostInput.freeList[i] = i;
        hostInput.syncEpochs[i] = -1;
        hostInput.taskIds[i] = 0;
    }
    hostInput.syncEpochs[0] = 0;
    hostInput.taskIds[0] = 1;
}

inline void setHostStencil2() {
    int * a = hostMem;
    int s = num_stencil_items;
    hostInput.freeList = a;
    hostInput.taskIds = (a += s);
    hostInput.syncEpochs = (a += s);
    hostInput.parentIds = (a += s);
    hostInput.val0s = (a += s);
    hostInput.val1s = (a += s);
    hostInput.val2s = (a += s);
    hostInput.val3s = (a += s);
    hostInput.val4s = (a += s);
}

inline void getTaskTail() {
    int tmp;
    hostInput.taskTail = (volatile int *) clEnqueueMapBuffer(command_queue,
                                            cilkInput.taskTail,
                                            CL_TRUE,
                                            CL_MAP_READ | CL_MAP_WRITE,
                                            0,
                                            4,
                                            0,
                                            (const cl_event *) NULL,
                                            (cl_event *) NULL,
                                            &tmp);
    //clFinish(command_queue);
}

inline void getFreeTail() {
    int tmp;
    hostInput.freeTail = (volatile int *) clEnqueueMapBuffer(command_queue,
                                            cilkInput.freeTail,
                                            CL_TRUE,
                                            CL_MAP_READ | CL_MAP_WRITE,
                                            0,
                                            4,
                                            0,
                                            (const cl_event *) NULL,
                                            (cl_event *) NULL,
                                            &tmp);
    //clFinish(command_queue);
}

inline void freeTaskTail() {
    clEnqueueUnmapMemObject(command_queue,
                            cilkInput.taskTail,
                            hostInput.taskTail,
                            0,
                            NULL,
                            NULL);
    //clFinish(command_queue);
}

inline void freeFreeTail() {
    clEnqueueUnmapMemObject(command_queue,
                            cilkInput.freeTail,
                            hostInput.freeTail,
                            0,
                            NULL,
                            NULL);
    //clFinish(command_queue);
}

const char * getError(int input) {
    int errorCode = input;
    switch (errorCode) {
        case CL_DEVICE_NOT_FOUND:
            return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE:
            return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE:
            return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES:
            return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY:
            return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP:
            return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH:
            return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE:
            return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE:
            return "CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_INVALID_VALUE:
            return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE:
            return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM:
            return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE:
            return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT:
            return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES:
            return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE:
            return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR:
            return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT:
            return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE:
             return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER:
            return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY:
            return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS:
            return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM:
            return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME:
            return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION:
            return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL:
            return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX:
            return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE:
            return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE:
            return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS:
            return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION:
            return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE:
            return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE:
            return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET:
            return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST:
            return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT:
            return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION:
            return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT:
            return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE:
            return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL:
            return "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return "CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
            return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
        case CL_PLATFORM_NOT_FOUND_KHR:
            return "CL_PLATFORM_NOT_FOUND_KHR";
        case CL_DEVICE_PARTITION_FAILED_EXT:
            return "CL_DEVICE_PARTITION_FAILED_EXT";
        case CL_INVALID_PARTITION_COUNT_EXT:
            return "CL_INVALID_PARTITION_COUNT_EXT";
        default:
            return "unknown error code";
    }
    return "unknown error code";
} 
/*
 * call before runStencil
 * num_items: number of stencil items
 * input_maze: Maze object to be solved
 */
inline void initStencil(int argc, int num_items, RectMaze input_maze, char **argv) {
    num_stencil_items = num_items;

    global_size[0] = num_stencil_items;

    local_size[0] = (num_stencil_items > 256) ? 256 : num_stencil_items;

    stencilSize = STENCIL_SIZE(num_stencil_items);
    input_array_size = input_maze.total_size();

    start_vertex = input_maze.start_vertex();
    dest_vertex = input_maze.dest_vertex();

    /* source code for kernel */
    const int SOURCE_COUNT = 1;
    char** all_source_str = new char*[SOURCE_COUNT];
    size_t* lengths = new size_t[SOURCE_COUNT];

    FILE *fp = fopen("bfs.cl", "r");
    char *cl_source_str = new char[MAX_SOURCE_SIZE];
    lengths[0] = fread(cl_source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    all_source_str[0] = cl_source_str;
/*  // uncommenting this and changing index of lengths and all_source_str in line 405, 407 does not work
    fp = fopen("index.h", "r");
    char *maze_header_str = new char[MAX_SOURCE_SIZE];
    lengths[0] = fread(maze_header_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    all_source_str[0] = maze_header_str;
*/

    /* build platform. ret stores error messages */
    cl_int ret;

    ret = clGetPlatformIDs(1,
                           &platform_id,
                           &ret_num_platforms);

    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;
#ifdef CPUDEBUG
    ret = clGetDeviceIDs(platform_id,
                         CL_DEVICE_TYPE_CPU/*DEFAULT*//*CPU*/,
                         1,
                         &device_id,
                         &ret_num_devices);
#else
    ret = clGetDeviceIDs(platform_id,
                         CL_DEVICE_TYPE_GPU/*DEFAULT*//*GPU*/,
                         1,
                         &device_id,
                         &ret_num_devices);
#endif

    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    context = clCreateContext(NULL,
                              1,
                              &device_id,
                              NULL,
			      NULL,
                              &ret);

    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    command_queue = clCreateCommandQueue(context,
                                         device_id,
                                         0,
                                         &ret);
    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    program = clCreateProgramWithSource(context,
                                        SOURCE_COUNT,      // number of sources
                                        all_source_str,    // array of source
                                        lengths,           // length of each source
                                        &ret);
    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    ret = clBuildProgram(program,
                         0,
                         0,
                         NULL,/*"-g",*/
                         NULL,
                         NULL);

    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    //hostMem = (int *) calloc(1, stencilSize);

    stencilMem = clCreateBuffer(context,
                                CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                                stencilSize,
                                NULL,//hostMem,
                                &ret);

    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    kernel = clCreateKernel(program,
                            "bfsKernel",
                            &ret);

    cleanKernel = clCreateKernel(program,
                                 "cleanKernel",
                                 &ret);

    setCilkStencil();

    hostMem = (int *) clEnqueueMapBuffer(command_queue,
                                         stencilMem,
                                         CL_TRUE,
                                         CL_MAP_READ | CL_MAP_WRITE,
                                         0,
                                         stencilSize,
                                         0,
                                         NULL,
                                         NULL,
                                         &ret);

    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    clFinish(command_queue);

    memset(hostMem, 1, stencilSize);


    //hostInput.taskTail = (volatile int *) malloc(4);
    //hostInput.freeTail = (volatile int *) malloc(4);
    getTaskTail();
    getFreeTail();
    /* for hostInput */
    setHostStencil();
    freeTaskTail();
    freeFreeTail();
    //printQueue(hostInput);
    //printDevQueue(cilkInput);

    ret = clEnqueueUnmapMemObject(command_queue,
                                  stencilMem,
                                  hostMem,
                                  0,
                                  NULL,
                                  NULL);

    if (ret)
        std::cout << __LINE__ << ": " << getError(ret) << std::endl;

    clFinish(command_queue);
}

inline void runStencil() {
    //std::cout << "starting..." << std::endl;
    // epoch starts at one since bfs has a double spawn
    int * epoch = new int(1);
    epoch[0] = 0;
    cl_int ret;
    getTaskTail();
    //std::cout << "got Tail" << std::endl;
    while (epoch[0] != -1) {
        int oldTaskTail = hostInput.taskTail[0];
        if (oldTaskTail > (num_stencil_items/ 2)) {
            //std::cout << "Cleaning up stencil" << std::endl;
            hostInput.taskTail = 0;
            getFreeTail();
            hostInput.freeTail = num_stencil_items - 1;
            freeTaskTail();
            freeFreeTail();

	    /* set arguments for function "cleanKernel" */
            ret = clSetKernelArg(cleanKernel,
                                 0,
                                 sizeof(cl_mem),
                                 &(stencilMem));

            ret = clSetKernelArg(cleanKernel,
                                 1,
                                 sizeof(cl_mem),
                                 &(cilkInput.taskTail));

            ret = clSetKernelArg(cleanKernel,
                                 2,
                                 sizeof(cl_mem),
                                 &(cilkInput.freeTail));

            clEnqueueNDRangeKernel(command_queue,
                                   cleanKernel,
                                   1,
                                   NULL,
                                   global_size,
                                   local_size,
                                   0,
                                   (const cl_event *) NULL,
                                   (cl_event *) NULL);
            getTaskTail();

        }//asm volatile ("":::"memory");
        //if (epoch[0] > 100)
        //    break;
        //clFinish(command_queue);
        freeTaskTail();
        //std::cout << "freed Tail" << std::endl;
	
        /* set arguments for bfsKernel */
        ret = clSetKernelArg(kernel,
                             0,
                             sizeof(int),
                             epoch);
        if (ret)
            std::cout << "set epcoh " << getError(ret) << std::endl;

#if 0
        ret = clSetKernelArg(kernel,
                             1,
                             sizeof(CilkStencil),
                             &(cilkInput));
        if (ret)
            std::cout << "set epcoh " << getError(ret) << std::endl; a: stencil memory
#endif
        ret = clSetKernelArg(kernel,
                             1,
                             sizeof(cl_mem),
                             &(stencilMem));

        if (ret)
            std::cout << "set stencil " << getError(ret) << std::endl;

        ret = clSetKernelArg(kernel,
                             2,
                             sizeof(cl_mem),
                             &(cilkInput.taskTail));
        if (ret)
            std::cout << "set stencil " << getError(ret) << std::endl;

        ret = clSetKernelArg(kernel,
                             3,
                             sizeof(cl_mem),
                             &(cilkInput.freeTail));

        if (ret)
            std::cout << "set stencil " << getError(ret) << std::endl;

        ret = clSetKernelArg(kernel,
                             4,
                             sizeof(cl_mem),
                             &inArray);

        if (ret)
            std::cout << "inArray " << getError(ret) << std::endl;

        ret = clEnqueueNDRangeKernel(command_queue,
                                     kernel,
                                     1,
                                     NULL,
                                     global_size,
                                     local_size,
                                     0,
                                     (const cl_event *) NULL,
                                     (cl_event *) NULL);
        //std::cout<<"Kernel Lauched"<< std::endl;

        getTaskTail();

        //std::cout << "oldTasktail = "<< oldTaskTail << std::endl;
        //std::cout << "hostInput.taskTail[0] = "<<  hostInput.taskTail[0]<< std::endl;
        //std::cout << "epoch is" << epoch[0] << std::endl;
        if (oldTaskTail == hostInput.taskTail[0]) {
            break;
        } else {
            epoch[0]++;
        }
        //std::cout << "epoch is" << epoch[0] << std::endl;
    }
    hostMem = (int *) clEnqueueMapBuffer(command_queue,
                                         stencilMem,
                                         CL_TRUE,
                                         CL_MAP_READ | CL_MAP_WRITE,
                                         0,
                                         stencilSize,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL);
    clFinish(command_queue);
}

inline void printResult() {
    
    int ret;
    hostArray = (int *) clEnqueueMapBuffer(command_queue,
                                           inArray,
                                           CL_TRUE,
                                           CL_MAP_READ | CL_MAP_WRITE,
                                           0,
                                           input_array_size * sizeof(int),
                                           0,
                                           NULL,
                                           NULL,
                                           &ret);
    
    for (int i = 0; i < input_array_size; i++)
	    std::cout << hostArray[i] << ' ';
    std::cout << std::endl;

    /* debug: print hostMem */
    hostMem = (int *) clEnqueueMapBuffer(command_queue,
                                         stencilMem,
                                         CL_TRUE,
                                         CL_MAP_READ | CL_MAP_WRITE,
                                         0,
                                         stencilSize,
                                         0,
                                         NULL,
                                         NULL,
                                         NULL);
    setHostStencil2();

//    for (int i = 0; i < 10; i++)
//	    std::cout << hostInput.val4s[i] << " ";
    for (int i = 0; i < 20; i++)
	    std::cout << hostInput.val0s[i] << " " << hostInput.val1s[i] << std::endl;
    std::cout << std::endl;
}

inline void cleanStencil() {
    clReleaseMemObject(stencilMem);
}


#endif // _CLIK_BFS_H_
