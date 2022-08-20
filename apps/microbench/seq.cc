#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include <chrono>
#include <iostream>
#include <thread>

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

const char *argv_default[] = {"", "w", "r"};

// extern bool pusnow_enable;
int main(int argc, const char *argv[]) {
    char *buffer = (char *)mmap(NULL, LENGTH, PROT_READ | PROT_WRITE,
                                MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (argc == 1) {
        argc = 3;
        argv = argv_default;
    }

    bool is_read[argc - 1];

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == 'r') {
            is_read[i - 1] = true;
            printf("read,");
        } else {
            is_read[i - 1] = false;
            printf("write,");
        }
    }
    printf("\n");

    int ret[argc - 1];
    std::chrono::high_resolution_clock::time_point time_from[argc - 1];
    std::chrono::high_resolution_clock::time_point time_to[argc - 1];

    printf("buffer: %p (%llx)\n", buffer, LENGTH);

    // population
    std::cout << "Populating..." << std::endl;
    write_workload(0, buffer, buffer + LENGTH);
    int id;
    for (id = 0; id < argc - 1; ++id) {
        std::cout << "Sleeping (10s)... step: " << id << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "Starting step: " << id << std::endl;

        time_from[id] = std::chrono::high_resolution_clock::now();
        ret[id] = is_read[id] ? read_workload(0, buffer, buffer + LENGTH)
                              : write_workload(0, buffer, buffer + LENGTH);

        time_to[id] = std::chrono::high_resolution_clock::now();
    }

    for (int id = 0; id < argc - 1; ++id) {
        printf("ret[%d]: %d\n", id, ret[id]);
    }
    for (int id = 0; id < argc - 1; ++id) {
        auto tm = std::chrono::duration_cast<std::chrono::microseconds>(
                      time_to[id] - time_from[id])
                      .count();
        std::cout << (is_read[id] ? "read: " : "write: ") << tm << " us"
                  << std::endl;
        std::cout << (is_read[id] ? "read (avg): " : "write (avg): ")
                  << tm / (LENGTH / 4194304) << " ns" << std::endl;
    }

    return 0;
}