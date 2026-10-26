# Experimental Observations & Synchronization Analysis

This document provides a detailed technical analysis of the synchronization experiments conducted for **PGC Experiment 5: Synchronization Mechanisms in Parallel Programs**.

---

## 1. Summary of Observed Mechanisms

| Mechanism | Target Primitive | Correctness Guarantee | Primary Bottleneck / Cost | Best Use Case |
| :--- | :--- | :--- | :--- | :--- |
| **None (Data Race)** | None | **Fail** (Data lost) | None (fastest execution) | Read-only operations or thread-private data |
| **Critical Section** | `#pragma omp critical` | **Pass** (100% correct) | Serialization, thread queuing | Multi-line updates, complex critical blocks |
| **Atomic Operation** | `#pragma omp atomic` | **Pass** (100% correct) | Memory bus / cache contention | Simple scalar updates (`counter++`, accumulator) |
| **OpenMP Lock** | `omp_lock_t` | **Pass** (100% correct) | Lock acquisition/release overhead | Dynamic or non-lexical mutual exclusion |
| **Barrier** | `#pragma omp barrier` | **N/A** (Phase sync) | Idling until slowest thread finishes | Multi-stage algorithms, stencil iterations |
| **POSIX Semaphore** | `sem_t` (binary) | **Pass** (100% correct) | Context switching & OS syscalls | Inter-process sync, signaling, pthreads |

---

## 2. In-Depth Analysis of Individual Mechanisms

### 2.1 The Data Race (Unsynchronized Shared Access)
In `race.c`, multiple threads concurrently perform `counter++` without coordination:
```c
#pragma omp parallel for
for (long i = 0; i < 10000000; i++) {
    counter++;
}
```
- **Disassembly Level**: An increment is not atomic at the hardware level. It translates into:
  1. `mov (%rax), %edx` (Read memory into register)
  2. `add $0x1, %edx`   (Modify register value)
  3. `mov %edx, (%rax)` (Write register back to memory)
- **Interleaving**: If Thread A reads `counter = 50`, and before it writes `51`, Thread B also reads `50`, both threads will compute `51` and store `51`. Two logical increments occurred, but the stored value increased by only 1.
- **Experimental Verification**:
  - In the Ubuntu VM run, 10,000,000 iterations with 8 threads produced `Final counter = 3407513`—a catastrophic **65.9% loss of updates**.
  - In the single `./race` execution, `Final counter = 7251257` (27.5% lost updates).
  - Unsynchronized execution is fast because no synchronization instructions or bus locks are issued, but the computation is completely invalid.

### 2.2 Critical Section (`#pragma omp critical`)
- **Mechanism**: The compiler generates runtime calls (`GOMP_critical_start()` and `GOMP_critical_end()`) wrapping the enclosed block.
- **Behavior**: All threads are serialized. While one thread increments `counter`, all remaining threads are forced into a wait queue.
- **Overhead**:
  - At 1 thread: 0.1127s (no contention, just entry/exit overhead).
  - At 8 threads: 0.2852s on Ubuntu VM (high lock contention).
- **Conclusion**: Correctness is guaranteed (10,000,000), but performance degrades as more threads are added. Adding threads hurts throughput rather than helping it.

### 2.3 Atomic Operations (`#pragma omp atomic`)
- **Mechanism**: The compiler emits hardware-supported atomic primitives (e.g., `lock addl $1, (%rdi)` on x86-64).
- **Behavior**: The CPU hardware locks the relevant cache line via the MESI/MOESI cache coherence protocol for the single instruction duration.
- **Overhead**:
  - At 1 thread: 0.0681s (40% faster than Critical).
  - At 8 threads: 0.1760s (38% faster than Critical).
- **Scalability**: Because no full lock/unlock runtime routines are executed and thread scheduling is not blocked, atomics exhibit superior throughput for scalar updates.

### 2.4 Barrier Synchronization (`#pragma omp barrier`)
- **Mechanism**: Enforces a strict temporal ordering across all threads in the parallel team.
- **Behavior**:
  - Pre-barrier messages (`Thread X before barrier`) are printed in non-deterministic order as threads spawn.
  - No thread can proceed to print `Thread X after barrier` until *every* thread in the team has arrived at the barrier.
- **Key Understanding**: A barrier does *not* protect a shared accumulator from concurrent writes; its purpose is phase synchronization between algorithmic stages.

### 2.5 POSIX Semaphore (`sem_t`)
- **Mechanism**: An OS-managed synchronization object initialized with `sem_init(&semaphore, 0, 1)`.
- **Behavior**:
  - `sem_wait()` decrements the semaphore counter; if 0, the thread is descheduled by the operating system scheduler.
  - `sem_post()` increments the counter and wakes the waiting thread.
- **Measured Result**: Two POSIX threads executing 100,000 increments each yielded `Final counter = 200000`, proving mutual exclusion.

---

## 3. Scalability and Contention Findings

1. **Amdahl's Law in Synchronization**: In fine-grained increment benchmarks, the entire loop body is a critical section. The serial portion ($s$) approaches 1.0. Under Amdahl's Law, when $s \to 1.0$, speedup is bounded by 1, and real-world synchronization overhead causes execution time to increase with more threads.
2. **False Sharing and Cache Bouncing**: In atomic operations, threads repeatedly attempt to acquire ownership of the cache line containing `counter`. As thread counts increase, cache-line invalidation traffic between CPU core caches increases, explaining why time increases from 1 to 2 threads before stabilizing.
3. **Platform Differences**:
   - On Linux (Ubuntu 22.04 LTS), the glibc OpenMP implementation leverages Linux `futex` (fast userspace mutex), keeping lock overhead low.
   - On Windows (MinGW GCC), user-to-kernel event transitions under high thread contention can magnify `omp_lock_t` wait times.
