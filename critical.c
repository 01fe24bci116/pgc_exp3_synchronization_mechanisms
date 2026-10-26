/*
 * critical.c - Synchronization using OpenMP Critical Section
 *
 * Objective:
 *   Demonstrates mutual exclusion using OpenMP's '#pragma omp critical' directive
 *   to ensure only one thread increments the shared counter at any given time.
 *
 * Concepts:
 *   - Critical Section: A block of code protected such that only one thread
 *     may execute it at a time. All other threads attempting to enter must wait.
 *   - Mutual Exclusion: Prevents race conditions by serializing access to the
 *     shared resource (counter++).
 *   - Trade-off: Guarantees correctness (Final counter = 10,000,000), but
 *     incurs significant synchronization overhead and serial bottleneck due
 *     to lock contention as thread count scales.
 *
 * Compilation:
 *   gcc critical.c -fopenmp -o critical
 *
 * Execution:
 *   ./critical
 */

#include <stdio.h>
#include <omp.h>

#define NUM_ITERATIONS 10000000L

int main(void) {
    long counter = 0;

    #pragma omp parallel for
    for (long i = 0; i < NUM_ITERATIONS; i++) {
        /*
         * #pragma omp critical ensures that the enclosed compound block
         * is executed by a single thread at a time across the entire team.
         */
        #pragma omp critical
        {
            counter++;
        }
    }

    /*
     * Because mutual exclusion is enforced on every increment,
     * no updates are lost. The final counter is guaranteed to be 10,000,000.
     */
    printf("Final counter = %ld\n", counter);

    return 0;
}
