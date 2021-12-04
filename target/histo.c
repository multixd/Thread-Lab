#include "histo.h"

/*  ============= 

Add corresponding locks/semaphores and any other global variables here
Sample declarations:
sem_t mutex; //don't for get to initialize in main
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_barrier_t barrier; //don't forget to initialize in main
    pthread_mutex_t locks[BUCKET_SIZE]; //don't forget to initialize in main

=============  */


void init() {
    /*  ============= 
    add initialization code for locks, or other global variables defined earlier

    the init() function will be called once at the beginning of each trial,
    as shown in the pseudocode below

    for (int i = 0; i < num_trials; i++) {
        init();
        test_results.append(run_tests(all_cases_or_a_particular_case));
    }
    =============  */
    
}

/************ Test Case 1 ************/


void* compute_histogram_case1(void* input) {
    /*  =============
    this is the thread worker function that will be called for test case 1
    you can assume that there are T1N data points T1B histogram buckets

    This function will be called for each of the NTHREADS (8) worker threads

    The data array pointer (int* data) and the histogram array pointer (int* hist)
    will be the same for each thread. 

    thread_id would be different for each thread: each of the 8 worker 
    threads will get a different thread_id ranging [0, 7] (inclusive on both sides)

    The function given below is effectively only "single-threaded", because only one
    thread (with thread_id 0) does all the work, while all the other threads returns
    immediately. As a result, the performance is not that good.

    You should modify this function to effectively use the pool of the 8 worker 
    threads to compute the histogram faster.
    =============  */
    HistogramArgs* histArgs = (HistogramArgs*)input;
    int* data = (int*)(histArgs->data);
    int* hist = (int*)(histArgs->hist);
    const int thread_id = histArgs->id;

    if (thread_id == 0) {
        for (int j = 0; j < T1N; j++) {
            hist[data[j] % T1B]++;
        }
    }
    return NULL;
}

/*  =============

This commented out function tries to divide up the data into
blocks, one for each thread. However, there is a race! (where?)

Try it out!

void* compute_histogram_case1_RACE_CONDITION(void* input) {
    HistogramArgs* histArgs = (HistogramArgs*)input;
    int* data = (int*)(histArgs->data);
    int* hist = (int*)(histArgs->hist);
    const int thread_id = histArgs->id;

    const int STEP = T1N / NTHREADS;
    const int start = thread_id * STEP;
    const int end = start + STEP;

    for (int i = start; i < end; i++) {
        hist[data[i] % T1B]++;
    }

    return NULL;
}

=============  */

/************ Test Case 2 ************/

void* compute_histogram_case2(void* input) {
    /*  =============
    this function will be called for test case 2
    you can assume that there are T2N data points T2B histogram buckets

    This function will be called for each of the NTHREADS (i.e., 8) worker threads

    The data array pointer (int* data) and the histogram array pointer (int* hist)
    will be the same for each thread. 

    thread_id would be different for each thread: each of the 8 worker 
    threads will get a different thread_id ranging [0, 7] (inclusive on both sides)

    The function given below is effectively only "single-threaded", because only one
    thread (with thread_id 0) does all the work, while all the other threads returns
    immediately. As a result, the performance is not that good.

    You should modify this function to effectively use the pool of the 8 worker 
    threads to compute the histogram faster.
    =============  */
    HistogramArgs* histArgs = (HistogramArgs*)input;
    int* data = (int*)(histArgs->data);
    int* hist = (int*)(histArgs->hist);
    const int thread_id = histArgs->id;

    if (thread_id == 0) {
        for (int j = 0; j < T2N; j++) {
            hist[data[j] % T2B]++;
        }
    }
    return NULL;
}