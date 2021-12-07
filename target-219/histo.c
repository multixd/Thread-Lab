#include "histo.h"
#include "semaphore.h"

sem_t mutex;
int case2[8][T2B];
/*  ============= 

Add corresponding locks/semaphores and any other global variables here
Sample declarations:
sem_t mutex; //don't for get to initialize in main
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_barrier_t barrier; //don't forget to initialize in main
    pthread_mutex_t locks[BUCKET_SIZE]; //don't forget to initialize in main

=============  */


void init() {
	sem_init(&mutex, 0, 1);
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

    const int STEP = T1N / NTHREADS;
    const int start = thread_id * STEP;
    const int end = start + STEP;
	int local_Hist[8] = {0,0,0,0,0,0,0,0};

    for (int i = start; i < end; i++) {
   	local_Hist[data[i] % T1B]++;
    }
for (int i = 0; i < 8; ++i) {
	sem_wait(&mutex);
	hist[i] += local_Hist[i];
	sem_post(&mutex);
}
                                                      
/*for(int i = 0; i < 8; i++) {
 printf("%d ", i);
        printf("%d ", local_Hist[i]);
    printf("\n");
}*/
/*
for(int i = 0; i < sizeof(local_Hist); i++) {
 printf("%d ", i);
        printf("%d ", local_Hist[i]);
    printf("\n");
}
if(thread_id == 1){
for(int i = 0; i < sizeof(local_Hist); i++) {
 printf("%d ", i);
        printf("%d ", hist[i]);
    printf("\n");
}
}*/

/*int static mutex = 1;
for (int i = 0; i < 8; ++i) {
	while(!__sync_bool_compare_and_swap(&mutex,1,0)) {
//		__sync_fetch_and_add(&hist[i] ,local_Hist[i] );
		hist[i] += local_Hist[i];
		__sync_bool_compare_and_swap(&mutex,0,1);
	}
}
*/
return NULL;


/*  =============

This commented out function tries to divide up the data into
blocks, one for each thread. However, there is a race! (where?)

Try it out!
*/

}
/*
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
    const int STEP = T2N / NTHREADS;
    const int start = thread_id * STEP;
    const int end = start + STEP;

 for (int i = start; i < end; i++) {
        case2[thread_id][data[i] % T2B]++;
 }

 for (int i = 0; i < T2B; ++i) {
               __sync_fetch_and_add(&hist[i] ,case2[thread_id][i] );
 }



    return NULL;
}
