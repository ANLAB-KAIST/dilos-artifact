#include <ddc/eviction.h>
#include <ddc/mman.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <iostream>
#define NUM_THREAD 1
#define ITER 1
#define LENGTH (20ULL << 30)

static inline uint64_t rdtsc(void) {
    uint32_t a, d;
    asm volatile("rdtsc" : "=a"(a), "=d"(d));
    return ((uint64_t)a) | (((uint64_t)d) << 32);
}

int read_workload(int i, char *start, char *end) {
    int sum = 0;
    for (int j = 0; j < ITER; j++) {
        uint64_t diff_sum = 0;
        for (char *c = start; c < end; c += 4096) {
            auto t1 = rdtsc();
            sum += *c;
            auto t2 = rdtsc();
            diff_sum += t2 - t1;
        }
        auto tt = diff_sum / ((end - start) / 4096);
        std::cout << "per read: " << tt << std::endl;
    }
    return sum;
}
int counter = 0;
int write_workload(int i, char *start, char *end) {
    int sum = 0;
    for (int j = 0; j < ITER; j++) {
        for (char *c = start; c < end; c += 4096) {
            sum += counter;
            *(int *)c = counter++;
        }
    }
    return sum;
}

static uint64_t mask = 0;

// extern bool pusnow_enable;
int main(int argc, char *argv[]) {
    char *buffer = (char *)mmap(NULL, LENGTH, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE | MAP_DDC, -1, 0);

    printf("buffer: %p (%llx)\n", buffer, LENGTH);

    // population
    write_workload(0, buffer, buffer + LENGTH);

    int ret1 = write_workload(0, buffer, buffer + LENGTH);
    int ret2 = read_workload(0, buffer, buffer + LENGTH);

    return 0;
}