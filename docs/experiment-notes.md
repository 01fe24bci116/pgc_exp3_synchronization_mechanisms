# Experiment Notes: Synchronization Mechanisms in Parallel Programs

**Course / Lab:** Parallel & Grid Computing (PGC) — Experiment 5  
**Topic:** Shared-Memory Synchronization, Mutual Exclusion, and Performance Scalability  
**Target Environment:** Ubuntu 22.04 LTS running on VMware Workstation Pro  
**Language & Toolchain:** C (C11 / C99), GCC, OpenMP (libgomp), POSIX Threads (`pthread`), POSIX Semaphores (`semaphore.h`)

---

## 1. Objectives

1. Implement and investigate shared-memory race conditions in multi-threaded C programs using OpenMP.
2. Apply compiler-directed mutual exclusion using `#pragma omp critical`.
3. Apply hardware-assisted atomic increments using `#pragma omp atomic`.
4. Demonstrate thread phase synchronization and barrier barriers using `#pragma omp barrier`.
5. Implement low-level mutual exclusion using native POSIX threads and POSIX unnamed binary semaphores (`sem_t`).
6. Benchmark correctness, wall-clock execution time, and scalability across 1, 2, 4, and 8 threads for unsynchronized, critical section, atomic, and lock-based updates.

---

## 2. Experimental Setup

| Parameter | Specification | Notes |
| :--- | :--- | :--- |
| **Host System** | x86-64 Multicore Host | AMD / Intel multicore processor |
| **Hypervisor** | VMware Workstation Pro | Virtualization layer |
| **Guest OS** | Ubuntu 22.04 LTS (x86_64) | Linux kernel 5.15 / 6.x |
| **Compiler** | GCC (GNU Compiler Collection) | Includes `libgomp` OpenMP runtime |
| **Threading Libraries** | OpenMP & `libpthread` | Bundled with `build-essential` |
| **Workload Size** | 10,000,000 iterations | Standardized across synchronization benchmarks |
| **Thread Counts Tested**| 1, 2, 4, and 8 threads | Set via `omp_set_num_threads()` |

---

## 3. Program Descriptions & Key Commands

### 3.1 Unsynchronized Counter (`race.c`)
- **Purpose**: Demonstrates non-deterministic lost updates when threads concurrently read, modify, and write a shared variable without locks.
- **Build & Run**:
  ```bash
  gcc race.c -fopenmp -o race
  ./race
  ```
- **Observed Result**: `Final counter = 7251257` (Expected: 10,000,000). Updates were lost due to unsynchronized race conditions.

### 3.2 Critical Section (`critical.c`)
- **Purpose**: Serializes access to the increment block so only one thread executes it at any time.
- **Build & Run**:
  ```bash
  gcc critical.c -fopenmp -o critical
  ./critical
  ```
- **Observed Result**: `Final counter = 10000000` (100% correct).

### 3.3 Atomic Operation (`atomic.c`)
- **Purpose**: Replaces software lock acquisition with a single hardware atomic CPU instruction (`LOCK ADD` on x86-64).
- **Build & Run**:
  ```bash
  gcc atomic.c -fopenmp -o atomic
  ./atomic
  ```
- **Observed Result**: `Final counter = 10000000` (100% correct, lower latency than critical section).

### 3.4 Barrier Synchronization (`barrier.c`)
- **Purpose**: Synchronizes thread phases such that all threads arrive at the barrier before any thread proceeds past it.
- **Build & Run**:
  ```bash
  gcc barrier.c -fopenmp -o barrier
  ./barrier
  ```
- **Observed Result**: All threads print Phase 1 ("before barrier") before any thread prints Phase 2 ("after barrier").

### 3.5 POSIX Semaphore (`semaphore.c`)
- **Purpose**: Mutual exclusion using POSIX `sem_init`, `sem_wait`, `sem_post`, and `sem_destroy`.
- **Build & Run**:
  ```bash
  gcc semaphore.c -pthread -o semaphore
  ./semaphore
  ```
- **Observed Result**: `Final counter = 200000` across 2 threads executing 100,000 increments each.

### 3.6 Comprehensive Benchmark (`compare_sync.c`)
- **Purpose**: Direct side-by-side performance comparison across 1, 2, 4, and 8 threads.
- **Build & Run**:
  ```bash
  gcc compare_sync.c -fopenmp -o compare_sync
  ./compare_sync
  ```

---

## 4. Key Observations

1. **Race Conditions Degrade with Thread Count**: In the comparison benchmark, the unsynchronized counter achieved 10,000,000 at 1 thread (serial execution), but degraded to 6,088,479 at 2 threads, 6,205,502 at 4 threads, and 3,407,513 at 8 threads. The more threads contending without synchronization, the higher the collision rate and data loss.
2. **Atomic vs. Critical Scalability**:
   - `Atomic` consistently outperformed `Critical` across all thread counts:
     - 1 Thread: Atomic 0.0681s vs. Critical 0.1127s
     - 2 Threads: Atomic 0.1714s vs. Critical 0.2612s
     - 4 Threads: Atomic 0.1791s vs. Critical 0.2647s
     - 8 Threads: Atomic 0.1760s vs. Critical 0.2852s
   - This occurs because `critical` introduces full software lock acquisition and thread blocking, while `atomic` maps directly to hardware-level cache coherence instructions without OS thread descheduling.
3. **Synchronization Inversion (Negative Scaling)**: For micro-benchmarks consisting entirely of increments inside a shared critical section, adding more threads increases execution time rather than decreasing it. This is expected because the critical section cannot be parallelized; adding threads only increases lock contention and cache-line invalidation traffic.

---

## 5. Experimental Limitations

1. **Virtual Machine Preemption**: In a VMware virtual machine, the guest OS shares physical CPU cores with the host. CPU scheduling latency and vCPU preemption can introduce timing jitter between runs.
2. **Synthetic Bottleneck**: Incrementing a single shared integer 10 million times represents an extreme worst-case scenario for lock contention (100% serial critical section). Real-world applications typically perform substantial thread-local computation between synchronization points, yielding positive speedup.
3. **Compiler Optimizations**: Compiling with aggressive optimizations (`-O2` or `-O3`) without `volatile` can cause GCC to transform the loop into thread-local register additions or hoist the variable, potentially masking the race condition. Therefore, compiling with `-Wall -Wextra` without aggressive optimization is required for faithful pedagogical reproduction.

---

## 6. Conclusion

The experiment demonstrates that shared-memory parallel programming requires explicit synchronization to prevent data corruption. However, synchronization introduces trade-offs:
- **Unsynchronized updates** are the fastest but yield mathematically invalid and nondeterministic results.
- **Critical sections and locks** guarantee complete correctness for arbitrary code blocks but introduce substantial contention bottlenecks.
- **Atomic operations** provide the optimal balance for scalar updates by utilizing hardware-assisted CPU primitives, outperforming critical sections across all thread counts.
- **Barriers** effectively coordinate execution phases across threads without directly protecting shared variables.
