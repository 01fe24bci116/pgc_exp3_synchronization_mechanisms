/*
 * race.c - Demonstration of a Race Condition (Unsynchronized Counter)
 * 
 * Objective:
 *   Demonstrates data inconsistency when multiple OpenMP threads concurrently
 *   increment a shared counter without synchronization.
 *
 * Concepts:
 *   - Race Condition: Multiple threads execute read-modify-write operations
 *     (counter++) on a shared variable simultaneously without coordination.
 *   - The operation counter++ consists of three distinct CPU instructions:
 *       1. READ: Load current counter from memory into a register.
 *       2. MODIFY: Increment value inside the CPU register.
 *       3. WRITE: Store updated value back to shared memory.
 *     When multiple threads interleave these steps, concurrent writes overwrite
 *     each other, causing lost updates.
 *
 * Compilation:
 *   gcc race.c -fopenmp -o race
 *
 * Execution:
 *   ./race
 */

#include <stdio.h>
#include <omp.h>

#define NUM_ITERATIONS 10000000L

int main(void) {
    long counter = 0;

    /*
     * Unsynchronized parallel loop:
     * By default in OpenMP, variables declared outside the parallel region
     * (like 'counter') are shared among all threads.
     * No mutual exclusion is applied here.
     */
    #pragma omp parallel for
    for (long i = 0; i < NUM_ITERATIONS; i++) {
        counter++;
    }

    /*
     * Because increments are lost during concurrent execution, the final value
     * is non-deterministic and strictly less than NUM_ITERATIONS.
     */
    printf("Final counter = %ld\n", counter);

    return 0;
}
