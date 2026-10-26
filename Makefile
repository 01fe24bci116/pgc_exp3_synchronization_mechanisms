# Makefile for Synchronization Mechanisms in Parallel Programs
# Compatible with Ubuntu 22.04 LTS (Linux) and Windows (MinGW / MSYS2)

CC ?= gcc
CFLAGS ?= -Wall -Wextra
OMP_FLAGS = -fopenmp
PTHREAD_FLAGS = -pthread

TARGETS = race critical atomic barrier semaphore compare_sync

ifeq ($(OS),Windows_NT)
    CLEAN_CMD = -del /Q /F $(TARGETS) $(addsuffix .exe,$(TARGETS)) *.o 2>nul || exit 0
else
    CLEAN_CMD = rm -f $(TARGETS) *.o
endif

.PHONY: all clean race critical atomic barrier semaphore compare compare_sync

all: $(TARGETS)

race: race.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) race.c -o race

critical: critical.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) critical.c -o critical

atomic: atomic.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) atomic.c -o atomic

barrier: barrier.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) barrier.c -o barrier

semaphore: semaphore.c
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) semaphore.c -o semaphore

compare_sync: compare_sync.c
	$(CC) $(CFLAGS) $(OMP_FLAGS) compare_sync.c -o compare_sync

compare: compare_sync

clean:
	$(CLEAN_CMD)
