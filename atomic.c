/*
 * atomic.c - Synchronization using OpenMP Atomic Operation
 *
 * Objective:
 *   Demonstrates lightweight synchronization using OpenMP's '#pragma omp atomic'
 *   directive to safely increment a shared counter using hardware-level atomics.
 *
 * Concepts:
 *   - Atomic Operation: Directs the compiler to perform a specific memory
 *     location update (such as counter++) as an indivisible operation.
 *   - Hardware Acceleration: Instead of acquiring a high-overhead OS or runtime
 *     lock (as in critical sections), atomics leverage hardware instructions
 *     (e.g., LOCK XADD, Compare-And-Swap) supported directly by the CPU.
 *   - Trade-off: Guarantees correctness (Final counter = 10,000,000) with
 *     significantly lower latency and higher scalability than critical sections,
 *     but is restricted to basic scalar expressions (cannot protect multiple
 *     statements or complex code blocks).
 *
 * Compilation:
 *   gcc atomic.c -fopenmp -o atomic
 *
 * Execution:
 *   ./atomic
 */

#include <stdio.h>
#include <omp.h>

#define NUM_ITERATIONS 10000000L

int main(void) {
    long counter = 0;

    #pragma omp parallel for
    for (long i = 0; i < NUM_ITERATIONS; i++) {
        /*
         * #pragma omp atomic ensures that the specific memory location
         * 'counter' is updated atomically without full block lock contention.
         */
        #pragma omp atomic
        counter++;
    }

    /*
     * Guaranteed exact count without data race loss.
     */
    printf("Final counter = %ld\n", counter);

    return 0;
}
