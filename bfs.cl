#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable
#define QUICKSIZE 128
#define MERGESIZE 1024

//#include "index.h"

typedef struct {
    // Basic Stencil
    __global int * taskTail;
    __global int * freeTail;
    __global int * freeList;
    __global int * taskIds;
    __global int * syncEpochs;
    __global int * parentIds;
    // Applicattion Specific
    __global int * val0s;
    __global int * val1s;
    __global int * val2s;
    __global int * val3s;
    __global int * val4s;
} CilkStencil;

int addTask(CilkStencil queue, int epoch, int taskId, int arg0, int arg1, int arg2, int arg3, int arg4) {
    int idx = atomic_add(queue.taskTail, 1);
    idx %= get_global_size(0);
    queue.taskIds[idx] = taskId;
    queue.syncEpochs[idx] = epoch;
    queue.parentIds[idx] = get_global_id(0);
    queue.val0s[idx] = arg0;
    queue.val1s[idx] = arg1;
    queue.val2s[idx] = arg2;
    queue.val3s[idx] = arg3;
    queue.val4s[idx] = arg4;
    return idx;
}

int addSelf(CilkStencil queue, int epoch, int taskId, int arg0, int arg1, int arg2, int arg3, int arg4) {
    int idx = get_global_id(0);
    queue.taskIds[idx] = taskId;
    queue.syncEpochs[idx] = epoch;
    queue.val0s[idx] = arg0;
    queue.val1s[idx] = arg1;
    queue.val2s[idx] = arg2;
    queue.val3s[idx] = arg3;
    queue.val4s[idx] = arg4;
    return idx;
}

void emitValue(CilkStencil queue, int value) {
    queue.val0s[get_global_id(0)] = value;

}

void clean(CilkStencil queue, int target) {
    queue.syncEpochs[target] = -1;
}

/* constants related to the format of inArray */
#define WEIGHT_INDEX 0
#define HEIGHT_INDEX 1
#define START_INDEX 2
#define DEST_INDEX 4
#define MATRIX_INDEX 6

int DestReached(CilkStencil queue, __global int* input_data, int thread_id) {
    return (queue.val0s[thread_id] == input_data[DEST_INDEX] && 
            queue.val1s[thread_id] == input_data[DEST_INDEX]) ? 1 : 0;
}

int calc_index_by_row_col(int row, int col, __global int* input_data) {
    return MATRIX_INDEX + row * input_data[WEIGHT_INDEX] + col;
}

/* return 1 if the position has not been searched, otherwise 0 */
int is_new_position(int row, int col, CilkStencil queue, int thread_id) {
    for (int i = 0; i < get_global_size(0); i++) {
        if (row == queue.val0s[i] && col == queue.val1s[i]) {
	    return 0;
	}
    }
    return 1;
}

/*
 * a: stencil memory
 * array: inArray
 */
__kernel void bfsKernel(int epoch,
                        __global int * a,
                        __global int * taskTail,
                        __global int * freeTail,
                        __global int * array) {

    int tid = get_global_id(0);
    //if (tid == 0)
    //    printf("\n------ EPOCH:%d -------\n", epoch);
    CilkStencil queue;
    
    int s = get_global_size(0);
    queue.freeList = a;
    queue.taskIds = (a += s);
    queue.syncEpochs = (a += s);
    queue.parentIds = (a += s);
    queue.val0s = (a += s);
    queue.val1s = (a += s);
    queue.val2s = (a += s);
    queue.val3s = (a += s);
    queue.val4s = (a += s);
    queue.taskTail = taskTail;
    queue.freeTail = freeTail;
    int syncEpoch = queue.syncEpochs[tid];
    if (syncEpoch != epoch) return;
    
    int taskId = queue.taskIds[tid];
    if (taskId == 0) return;

    // cilkbfs
    if (taskId == 1) {
        int start_row = queue.val0s[tid];
        int start_col = queue.val1s[tid];
        if (DestReached(queue, array, tid)) {
            queue.val3s[tid] = 1;
            return;
        }
        queue.val4s[tid] = 100;
	if (start_row < 2)
	    addTask(queue, epoch+1, 1, start_row + 1, 1, 0, 0, 0);
	
        /*if ((start_row > 0) && (array[calc_index_by_row_col(start_row - 1, start_col, array)]) &&
		is_new_position(start_row - 1, start_col, queue, tid)) {
	    //queue.val4s[tid] = 1;
            addTask(queue, epoch + 1, 1, start_row - 1, start_col, 0, 0, 0);
        }*/
	/*if ((start_row < array[WEIGHT_INDEX] - 1) &&
	        (array[calc_index_by_row_col(start_row + 1, start_col, array)]) &&
		(is_new_position(start_row + 1, start_col, queue, tid))) {
            //queue.val4s[tid] = 2;
	    addTask(queue, epoch + 1, 1, start_row + 1, start_col, 0, 0, 0);
	}
        if ((start_col > 0) && (array[calc_index_by_row_col(start_row, start_col - 1, array)]) &&
		is_new_position(start_row, start_col - 1, queue, tid)) {
	    //queue.val4s[tid] = 4;
            addTask(queue, epoch + 1, 1, start_row, start_col - 1, 0, 0, 0);
        }
	if ((start_col < array[HEIGHT_INDEX] - 1) &&
	        (array[calc_index_by_row_col(start_row, start_col + 1, array)]) &&
		(is_new_position(start_row, start_col + 1, queue, tid))) {
            //queue.val4s[tid] = 8;
	    addTask(queue, epoch + 1, 1, start_row, start_col + 1, 0, 0, 0);
	}
*/
	//clean(queue, tid);
    }
    else if (taskId == 2) {

    }
}

__kernel void cleanKernel(__global int * a,
                          __global int * taskTail,
                          __global int * freeTail) {

    int tid = get_global_id(0);
    CilkStencil queue;
    int s = get_global_size(0);
    queue.freeList = a;
    queue.taskIds = (a += s);
    queue.syncEpochs = (a += s);
    queue.parentIds = (a += s);
    queue.val0s = (a += s);
    queue.val1s = (a += s);
    queue.val2s = (a += s);
    queue.val3s = (a += s);
    queue.val4s = (a += s);
    queue.taskTail = taskTail;
    queue.freeTail = freeTail;
    int sync = queue.syncEpochs[tid];
    int idx;
    if (sync > -1) {
        idx = atomic_add(queue.taskTail, 1);
    } else {
        idx = atomic_add(queue.freeTail, -1);
    }
    queue.freeList[idx] = tid;
}
