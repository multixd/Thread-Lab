#include "main.h"

#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "histo.h"

char hostname[HOST_NAME_MAX];

static struct option long_options[] = {
    //    {"direct-path",        no_argument,       nullptr, 'd',},
    {"input", required_argument, 0, 'i'},
    {"trials", required_argument, 0, 't'},
    {
        0,
        0,
        0,
        0,
    },
};

typedef void* (*Routine)(void*);

typedef struct {
    float orig_msec;
    float best_msec;
} TestPerformance;

typedef struct {
    int N;
    int B;
} HistogramTestParams;


void clear_3d(int Ni, int Nj, int Nk, float a[Ni][Nj][Nk]) {
    for (int i = 0; i < Ni; ++i) {
        for (int j = 0; j < Nj; ++j) {
            for (int k = 0; k < Nk; ++k) {
                a[i][j][k] = 0;
            }
        }
    }
}

void gen_3d(int Ni, int Nj, int Nk, float a[Ni][Nj][Nk]) {
    for (int i = 0; i < Ni; ++i) {
        for (int j = 0; j < Nj; ++j) {
            for (int k = 0; k < Nk; ++k) {
                a[i][j][k] = ((float)rand()) / (RAND_MAX / 8);
            }
        }
    }
}

void clear_1d(int N, int data[N]) {
    for (int i = 0; i < N; ++i) {
        data[i] = 0;
    }
}

void gen_1d(int N, int data[N]) {
    for (int i = 0; i < N; ++i) {
        data[i] = rand();
    }
}

static int max_errors_to_print = 5;

char check_3d(int Ni, int Nj, int Nk, float a[Ni][Nj][Nk],
              float a_check[Ni][Nj][Nk]) {
    int errors_printed = 0;
    char has_errors = 0;
    for (int i = 0; i < Ni; ++i) {
        for (int j = 0; j < Nj; ++j) {
            for (int k = 0; k < Nk; ++k) {
                if ((a[i][j][k] < (a_check[i][j][k] - 0.005)) ||
                    (a[i][j][k] > (a_check[i][j][k] + 0.005))) {
                    has_errors = 1;
                    if (errors_printed < max_errors_to_print) {
                        if (errors_printed == 0) printf("\n");
                        printf("Error on index: [%d][%d][%d].", i, j, k);
                        printf("Your output: %f, Correct output %f\n",
                               a[i][j][k], a_check[i][j][k]);
                        errors_printed++;
                    } else {
                        // printed too many errors already, just stop
                        if (max_errors_to_print != 0) {
                            printf("and many more errors likely exist...\n");
                        }
                        return 1;
                    }
                }
            }
        }
    }
    return has_errors;
}

char check_1d(int B, int a[B], int a_check[B]) {
    int errors_printed = 0;
    char has_errors = 0;
    for (int i = 0; i < B; ++i) {
        if (a[i] != a_check[i]) {
            has_errors = 1;
            if (errors_printed < max_errors_to_print) {
                if (errors_printed == 0) printf("\n");
                printf("Error on index: [%d]. ", i);
                printf("Your output: %d, Correct output %d\n", a[i],
                       a_check[i]);
                errors_printed++;
            } else {
                // printed too many errors already, just stop
                if (max_errors_to_print != 0) {
                    printf("and many more errors likely exist...\n");
                }
                return 1;
            }
        }
    }
    return has_errors;
}

void update_performance(uint64_t total_time, double computations,
                        TestPerformance* p) {
    double total_msec = total_time / 1000.0;

    float speedup = p->orig_msec / total_msec;
    p->best_msec = fmin(p->best_msec, total_msec);

    float ghz = 2.0;
    double clocks = total_time * 1000.0 * ghz;
    printf("  Runtime (msec): %0.1f, CPE: %0.3f, Speedup: %0.3f\n", total_msec,
           clocks / computations, speedup);
}

uint64_t spawn_threads_do_work(Routine func, void* argsPtrArray[NTHREADS]) {
    uint64_t start_time;
    pthread_t thread_id[NTHREADS];

    start_time = read_usec();

    int i;
    for (i = 0; i < NTHREADS; i++) {
        pthread_create(&thread_id[i], NULL, func, argsPtrArray[i]);
    }

    for (i = 0; i < NTHREADS; i++) {
        pthread_join(thread_id[i], NULL);
    }

    return (read_usec() - start_time);
}

float run_histogram_test(HistogramTestParams* p, char check_func,
                         char* is_broken, TestPerformance* test_performance,
                         Routine func) {
    int N = p->N, B = p->B;
    printf("Histogram test with N=%d, S=%d -- ", N, B);
    fflush(stdout);

    void* data = malloc(sizeof(int) * N);
    void* hist = malloc(sizeof(int) * B);
    void* hist_check = 0;

    // Generate the inputs
    gen_1d(N, data);
    clear_1d(B, hist);

    if (check_func) {
        hist_check = calloc(sizeof(int), B);
        clear_1d(B, hist_check);
        histogram_check(N, B, data, hist_check);
    }

    HistogramArgs histArgsArray[NTHREADS];
    void* argsPtrArray[NTHREADS] = {0};
    for (int i = 0; i < NTHREADS; i++) {
        histArgsArray[i].data = (void*)data;
        histArgsArray[i].hist = (void*)hist;
        histArgsArray[i].id = i;

        argsPtrArray[i] = &histArgsArray[i];
    }

    uint64_t total_time = spawn_threads_do_work(func, argsPtrArray);

    if (check_func) {
        if (check_1d(B, hist, hist_check)) {
            *is_broken = 1;
        } else {
            printf("no problems detected\n");
        }
    }

    update_performance(total_time, (double)N, test_performance);

    free(data);
    free(hist);
    if (hist_check) free(hist_check);

    return total_time;
}

#define NUM_HISTOGRAM_TESTS (2)
#define TOTAL_NUM_TESTS (NUM_HISTOGRAM_TESTS)

HistogramTestParams HistogramTests[NUM_HISTOGRAM_TESTS] = {{T1N, T1B},
                                                           {T2N, T2B}};

TestPerformance AllTestPerformance[TOTAL_NUM_TESTS] = {
    {200, 10000000.00}, {400, 10000000.00}};

char run_test(int i, char check_func) {
    char is_broken = 0;
    printf("Test %d, ", i);
    switch (i) {
        case 1:
            run_histogram_test(&(HistogramTests[0]), check_func, &is_broken,
                               &AllTestPerformance[i - 1],
                               compute_histogram_case1);
            break;
        case 2:
            run_histogram_test(&(HistogramTests[1]), check_func, &is_broken,
                               &AllTestPerformance[i - 1],
                               compute_histogram_case2);
            break;
        default:
            printf("WARNING! Expecting test case 1, or 2 \n");
            is_broken = 1;
            break;
    }
    return is_broken;
}

float interp(float s, float l, float lgrade, float h, float hgrade) {
    return (s - l) * (hgrade - lgrade) / (h - l) + lgrade;
}

float grade(float s) {
    if (s < 1) return 0;
    if (s < 3) return interp(s, 1, 0, 3, 100); // 1
    // higher than 4 gets Extra credit
    return 100;
}

int main(int argc, char** argv) {
    char do_all = 0;
    char check_func = 1;
    int test_case = 1;  // start with index 1, defaults to 1

    int opt;
    int num_trials = 1;

    int result = gethostname(hostname, HOST_NAME_MAX);
    if (result) {
        perror("gethostname");
        return EXIT_FAILURE;
    }

    char on_right_machine = 1;
    if (strcmp(hostname, "lnxsrv07.seas.ucla.edu") &&
        strcmp(hostname, "lnxsrv09.seas.ucla.edu")) {
        printf(
            "WARNING! You are not on lnxsrv07 or lnxsrv09, so results may not "
            "be "
            "valid!\n");
        printf("Your machine is %s.\n", hostname);
        on_right_machine = 0;
    }

    while ((opt = getopt_long(argc, argv, "i:t:", long_options, 0)) != -1) {
        switch (opt) {
            case 'i':
                if (*optarg == 'a')
                    do_all = 1;
                else {
                    test_case = atoi(optarg);
                }
                break;
            case 't':
                num_trials = atoi(optarg);
        }
    }

    printf("Num Trials: %d (change with --trials)\n", num_trials);
    if (do_all) {
        printf("Running all %d test cases!\n", TOTAL_NUM_TESTS);
    }

    int benchmarks_failed = 0;

    for (int t = 0; t < num_trials; ++t) {
        if (t != 0) printf("\n");

        init();
        printf("init() is called\n");
        if (do_all) {
            for (int i = 1; i < TOTAL_NUM_TESTS + 1; i++) {
                benchmarks_failed += run_test(i, check_func);
            }
        } else {
            run_test(test_case, check_func);
        }
    }

    if (benchmarks_failed) {
        printf("Number of Benchmarks FAILED: %d\n", benchmarks_failed);
        return 0;
    }

    if (do_all) {
        float gmean_speedup = 1;
        for (int i = 0; i < TOTAL_NUM_TESTS; i++) {
            double speedup = ((double)AllTestPerformance[i].orig_msec /
                              (double)AllTestPerformance[i].best_msec);
            gmean_speedup *= speedup;
        }
        gmean_speedup = pow(gmean_speedup, 1.0 / TOTAL_NUM_TESTS);
        printf("Geomean Speedup: %0.2f\n", gmean_speedup);
        if (on_right_machine) {
            printf("Grade: %0.1f\n", grade(gmean_speedup));
        } else {
            printf(
                "No grade given, because you're not on the right machine.\n");
        }
    } else {
        double speedup = ((double)AllTestPerformance[test_case - 1].orig_msec /
                          (double)AllTestPerformance[test_case - 1].best_msec);
        printf("Test Case %d Speedup: %0.2f\n", test_case, speedup);

        printf("No grade given, because only one test is run.\n");
        printf(" ... To see your grade, run all tests with \"-i a\"\n");
    }
    return 0;
}
