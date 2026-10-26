/*
 * semaphore.c - Mutual Exclusion using POSIX Threads and POSIX Binary Semaphore
 *
 * Objective:
 *   Demonstrates mutual exclusion in native POSIX threads (pthreads) using
 *   a POSIX unnamed binary semaphore (sem_t) to protect a shared counter.
 *
 * Concepts:
 *   - Semaphore: A synchronization variable maintaining an integer count.
 *   - Binary Semaphore: Initialized with a value of 1. Acts as a mutual exclusion
 *     lock (mutex).
 *   - sem_wait(): Decrements the semaphore value. If the value is 0, the calling
 *     thread blocks until another thread increments it.
 *   - sem_post(): Increments the semaphore value, waking up one waiting thread.
 *   - Overhead: Operating system system-call context switching when threads block.
 *
 * Compilation:
 *   gcc semaphore.c -pthread -o semaphore
 *
 * Execution:
 *   ./semaphore
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define ITERATIONS_PER_THREAD 100000

/* Shared variables */
sem_t semaphore;
int counter = 0;

/* Worker function executed by both threads */
void* work(void* arg) {
    (void)arg; /* Unused parameter */
    for (int i = 0; i < ITERATIONS_PER_THREAD; i++) {
        sem_wait(&semaphore);   /* Acquire lock (P-operation / decrement) */
        counter++;              /* Critical Section */
        sem_post(&semaphore);   /* Release lock (V-operation / increment) */
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    /*
     * Initialize unnamed semaphore:
     *   pshared = 0 (shared between threads of the process)
     *   value = 1   (binary semaphore for mutual exclusion)
     */
    if (sem_init(&semaphore, 0, 1) != 0) {
        perror("Failed to initialize semaphore");
        return 1;
    }

    /* Create two worker threads */
    if (pthread_create(&t1, NULL, work, NULL) != 0) {
        perror("Failed to create thread 1");
        sem_destroy(&semaphore);
        return 1;
    }
    if (pthread_create(&t2, NULL, work, NULL) != 0) {
        perror("Failed to create thread 2");
        pthread_join(t1, NULL);
        sem_destroy(&semaphore);
        return 1;
    }

    /* Wait for both threads to finish execution */
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    /* Output final counter value: expected 2 * 100000 = 200000 */
    printf("Final counter = %d\n", counter);

    /* Clean up semaphore resources */
    sem_destroy(&semaphore);

    return 0;
}
