/*
 * compare_sync.c - Benchmark and Scalability Comparison of Synchronization Mechanisms
 *
 * Objective:
 *   Evaluates correctness, execution time, and scalability across 1, 2, 4, and 8 threads
 *   for four synchronization strategies:
 *     1. None (Unsynchronized data race)
 *     2. Critical Section (#pragma omp critical)
 *     3. Atomic Operation (#pragma omp atomic)
 *     4. OpenMP Lock API (omp_lock_t)
 *
 * Concepts:
 *   - Scalability: How execution time changes as the number of threads increases.
 *   - Synchronization Overhead: The latency penalty introduced by cache-line bouncing,
 *     hardware bus locks, and OS lock contention.
 *   - Correctness: Verifies whether the final counter equals the expected total (10,000,000).
 *
 * Compilation:
 *   gcc compare_sync.c -fopenmp -o compare_sync
 *
 * Execution:
 *   ./compare_sync
 */

#include <stdio.h>
#include <omp.h>

#define N 10000000L

/* 1. Unsynchronized: No protection (demonstrates race condition) */
long run_none(int nthreads) {
    long counter = 0;
    omp_set_num_threads(nthreads);
    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        counter++;
    }
    return counter;
}

/* 2. Critical Section: High-level compiler directive for mutual exclusion */
long run_critical(int nthreads) {
    long counter = 0;
    omp_set_num_threads(nthreads);
    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        #pragma omp critical
        {
            counter++;
        }
    }
    return counter;
}

/* 3. Atomic Operation: Hardware-supported atomic increment */
long run_atomic(int nthreads) {
    long counter = 0;
    omp_set_num_threads(nthreads);
    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        #pragma omp atomic
        counter++;
    }
    return counter;
}

/* 4. OpenMP Lock: Explicit runtime lock (omp_lock_t) */
long run_lock(int nthreads) {
    long counter = 0;
    omp_lock_t lock;

    omp_init_lock(&lock);
    omp_set_num_threads(nthreads);

    #pragma omp parallel for
    for (long i = 0; i < N; i++) {
        omp_set_lock(&lock);
        counter++;
        omp_unset_lock(&lock);
    }

    omp_destroy_lock(&lock);
    return counter;
}

/* Helper to time execution, evaluate correctness, and print tabular results */
void run_and_report(const char *name, long (*fn)(int), int nthreads) {
    double start = omp_get_wtime();
    long result = fn(nthreads);
    double elapsed = omp_get_wtime() - start;
    const char *correctness = (result == N) ? "Correct" : "INCORRECT";

    printf("%-10s | %7d | %12ld | %10s | %10.4f\n",
           name, nthreads, result, correctness, elapsed);
    fflush(stdout);
}

int main(void) {
    int thread_counts[] = {1, 2, 4, 8};
    int num_configs = (int)(sizeof(thread_counts) / sizeof(thread_counts[0]));

    printf("===================================================================\n");
    printf("   Synchronization Mechanisms Benchmark (%ld Shared Increments)   \n", N);
    printf("===================================================================\n");
    printf("%-10s | %-7s | %-12s | %-10s | %-10s\n",
           "Mechanism", "Threads", "Final Count", "Correctness", "Time (s)");
    printf("-------------------------------------------------------------------\n");

    for (int i = 0; i < num_configs; i++) {
        int t = thread_counts[i];
        run_and_report("None",     run_none,     t);
        run_and_report("Critical", run_critical, t);
        run_and_report("Atomic",   run_atomic,   t);
        run_and_report("Lock",     run_lock,     t);
        printf("-------------------------------------------------------------------\n");
    }

    printf("\nExpected correct final count = %ld\n", N);
    printf("Key Insights:\n");
    printf("  1. 'None' is fastest but produces INCORRECT results due to data races.\n");
    printf("  2. 'Critical' and 'Lock' guarantee correctness but suffer severe lock contention.\n");
    printf("  3. 'Atomic' provides high performance with hardware-assisted operations.\n");
    printf("===================================================================\n");

    return 0;
}
