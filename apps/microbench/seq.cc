#include <ddc/eviction.h>
#include <ddc/mman.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <iostream>
#define NUM_THREAD 1
#define ITER 1
#define LENGTH (20ULL << 30)

int read_workload(int i, char *start, char *end) {
    int sum = 0;
    for (int j = 0; j < ITER; j++) {
        for (char *c = start; c < end; c += 4096) {
            sum += *c;
        }
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

static uint64_t _seq_ddc_eviction_mask(uintptr_t vaddr) {
    ;
    return mask;
}

// extern bool pusnow_enable;
int main(int argc, char *argv[]) {
    char *buffer = (char *)mmap(NULL, LENGTH, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE | MAP_DDC, -1, 0);

    printf("buffer: %p (%llx)\n", buffer, LENGTH);

    if (argc == 2) {
        mask = std::stoull(argv[1], NULL, 16);
        printf("mask: %lx\n", mask);
        ddc_register_eviction_mask(_seq_ddc_eviction_mask);
    }

    // population
    write_workload(0, buffer, buffer + LENGTH);

    auto t1 = std::chrono::high_resolution_clock::now();
    int ret1 = write_workload(0, buffer, buffer + LENGTH);
    auto t2 = std::chrono::high_resolution_clock::now();
    int ret2 = read_workload(0, buffer, buffer + LENGTH);
    auto t3 = std::chrono::high_resolution_clock::now();

    auto write_time =
        std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    auto read_time =
        std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
    std::cout << "ret1: " << ret1 << " ret2: " << ret2 << std::endl;
    std::cout << "write: " << write_time << " us" << std::endl;
    std::cout << "read: " << read_time << " us" << std::endl;
    return 0;
}