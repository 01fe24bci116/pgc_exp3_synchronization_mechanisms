/*
 * barrier.c - Phase Synchronization using OpenMP Barrier
 *
 * Objective:
 *   Demonstrates phase coordination across parallel threads using OpenMP's
 *   '#pragma omp barrier' directive.
 *
 * Concepts:
 *   - Barrier Synchronization: A synchronization point where every thread in
 *     the parallel team must arrive before any thread is permitted to proceed.
 *   - Phase Coordination: Barriers do not protect shared scalar data directly;
 *     instead, they ensure that Phase 1 (pre-barrier tasks) is fully completed
 *     by all threads before any thread initiates Phase 2 (post-barrier tasks).
 *   - Waiting Overhead: The execution time at the barrier is dictated by the
 *     slowest thread in the team. Faster threads remain idle until all reach
 *     the barrier.
 *
 * Compilation:
 *   gcc barrier.c -fopenmp -o barrier
 *
 * Execution:
 *   ./barrier
 */

#include <stdio.h>
#include <omp.h>

int main(void) {
    /*
     * Spawn an OpenMP parallel region.
     * All threads execute the block concurrently.
     */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();

        /* Phase 1: Pre-barrier execution */
        printf("Thread %d before barrier\n", tid);

        /*
         * Explicit barrier:
         * All threads must reach this directive before any thread can proceed.
         */
        #pragma omp barrier

        /* Phase 2: Post-barrier execution */
        printf("Thread %d after barrier\n", tid);
    }

    return 0;
}
