#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_extended_atomics : enable
#pragma OPENCL EXTENSION cl_amd_printf : enable
#define QUICKSIZE 128
#define MERGESIZE 1024

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

void printQueue(CilkStencil queue) {
    int tid = get_global_id(0);
    printf("taskTail[%d] = %x\n", tid, queue.taskTail[0]);
    printf("freeTail[%d] = %x\n", tid, queue.freeTail[0]);
    printf("freeList[%d] = %x\n", tid, queue.freeList[tid]);
    printf("taskIds[%d] = %x\n", tid, queue.taskIds[tid]);
    printf("syncEpochs[%d] = %x\n", tid, queue.syncEpochs[tid]);
    printf("parentIds[%d] = %x\n", tid, queue.parentIds[tid]);
    printf("val0s[%d] = %x\n", tid, queue.val0s[tid]);
    printf("val1s[%d] = %x\n", tid, queue.val1s[tid]);
    printf("val2s[%d] = %x\n", tid, queue.val2s[tid]);
    printf("val3s[%d] = %x\n", tid, queue.val3s[tid]);
    printf("val4s[%d] = %x\n", tid, queue.val4s[tid]);
    if (tid == 0) {
        printf("taskTail= %p\n", queue.taskTail);
        printf("freeTail %p\n", queue.freeTail);
        printf("freeList = %p\n", queue.freeList);
        printf("taskIds = %p\n", queue.taskIds);
        printf("syncEpochs = %p\n", queue.syncEpochs);
        printf("parentIds = %p\n", queue.parentIds);
        printf("val0s = %p\n", queue.val0s);
        printf("val1s = %p\n", queue.val1s);
        printf("val1s = %p\n", queue.val1s);
        printf("val2s = %p\n", queue.val1s);
        printf("val3s = %p\n", queue.val1s);
        printf("val4s = %p\n", queue.val1s);
    }
}

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

void seqquick(__global int * array, int low, int high) {
    int a, b, p, q;
    for(q = low + 1; q <= high; ++q) {
        a = array[q];
        for (p = q - 1; p >= low && (b = array[p]) > a; p--) {
            array[p + 1] = b;
        }
        array[p + 1] = a;
    }
}

int binsplit(__global int * array, int val, int low, int high) {
    int mid;
    //printf("low = %d, high = %d\n", low, high);
    while (low != high) {
        mid = low + ((high - low + 1) / 2);
        if (val <= array[mid]) {
            high = mid - 1;
        } else {
            low = mid;
        }
    }
    if (array[low] >= val) {
        //printf("low = %d\n", low-1);
        return (low - 1);
    } else {
        //printf("low = %d\n", low);
        return(low);
    }
}

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
    int taskId = queue.taskIds[tid];
    // cilkbfs
    if (taskId == 1) {
	int syncEpoch = queue.syncEpochs[tid];
        if (syncEpoch == epoch) {
            int A, B, C, D, tmpA, tmpB, tmpC, tmpD;
            int low  = queue.val0s[tid];
            int tmp  = queue.val1s[tid];
            int size  = queue.val2s[tid];
            if (size <= QUICKSIZE) {
                seqquick(array, low, low + size - 1);
                clean(queue, tid);
                return;
            }
            int quarter = size / 4;
            A = low;
            tmpA = tmp;
            B = A + quarter;
            tmpB = tmpA + quarter;
            C = B + quarter;
            tmpC = tmpB + quarter;
            D = C + quarter;
            tmpD = tmpC + quarter;
            int task0 = addTask(queue, epoch + 2, 1, A, tmpA, quarter, 0, 0);
            int task1 = addTask(queue, epoch + 2, 1, B, tmpB, quarter, 0, 0);
            int task2 = addTask(queue, epoch + 2, 1, C, tmpC, quarter, 0, 0);
            int task3 = addTask(queue, epoch + 2, 1, D, tmpD, size - 3 * quarter, 0, 0);
            //sync
            int task4 = addTask(queue, epoch + 1, 2, A, A + quarter - 1, B, B + quarter - 1, tmpA);
            int task5 = addTask(queue, epoch + 1, 2, C, C + quarter - 1, D, low + size - 1, tmpC);
            //sync
            int task6 = addSelf(queue, epoch, 2, tmpA, tmpC - 1, tmpC, tmpA + size - 1 , A);
        }
    } else if (taskId == 2) {
	int syncEpoch = queue.syncEpochs[tid];
        if (syncEpoch == epoch) {
            int low1    = queue.val0s[tid];
            int high1   = queue.val1s[tid];
            int low2    = queue.val2s[tid];
            int high2   = queue.val3s[tid];
            int lowdest = queue.val4s[tid];
            int split1, split2;
            int lowsize;
            //printf("\nmerge a[%d]\n", low1);

            //for (int i = 0; i <= (high1 - low1); i++) {
            //    printf("%d\t", array[low1 + i]);
            //}
            //printf("\nand a[%d]\n", low2);

            //for (int i = 0; i <= (high2 - low2); i++) {
            //    printf("%d\t", array[low2 + i]);
            //}
            //printf("\n\n");
            if (high2 - low2 > high1 - low1) {
                int tmp = high2;
                high2 = high1;
                high1 = tmp;
                tmp = low2;
                low2 = low1;
                low1 = tmp;
            }
            if (high1 < low1) {
                //printf("copy to a[%d] \n");
                //printf("array1 is empty\n");
                while(low2 <= high2) {
                    array[lowdest++] = array[low2++];
                    //printf("%d:%d\t", low2 - 1, array[lowdest - 1]);
                }
                //printf("\n\n");
                clean(queue, tid);
                return;
            }
            if ((high2 - low2) <= MERGESIZE) {
                //printf("merged to a[%d]\n", lowdest);
                while(low1 <= high1) {
                    int a1 = array[low1++];
                    if (high2 >= low2) {
                        for (int a2 = array[low2]; a2 < a1 ; a2 = array[low2]) {
                            array[lowdest++] = a2;
                            //printf("%d:%d\t", low2, a2);
                            low2++;
                            if (low2 > high2)
                                break;
                        }
                    }
                    array[lowdest++] = a1;
                    //printf("%d:%d\t", low1 - 1, a1);
                }
                while (high2 >= low2) {
                    array[lowdest++] = array[low2++];
                    //printf("%d:%d\t", low2 - 1, array[lowdest - 1]);
                }
                //printf("\n\n");
                clean(queue, tid);
                return;
            }
            split1 = ((high1 - low1 + 1) / 2) + low1;
            split2 = binsplit(array, array[split1], low2, high2);
            lowsize = split1 - low1 + split2 - low2;
            array[lowdest + lowsize + 1] = array[split1];
            //printf("\nsplit1 at:\na[%d] = %d\n", split1, array[split1]);
            //printf("\nsplit2 at:\na[%d] = %d\n", split2, array[split2]);
            int task0 = addTask(queue, epoch + 1, 2, low1, split1 - 1, low2, split2, lowdest);
            int task1 = addSelf(queue, epoch + 1, 2, split1 + 1, high1, split2 + 1, high2, lowdest + lowsize + 2);
        }
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
