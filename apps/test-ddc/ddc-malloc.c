#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define MALLOC_SIZE (8192)
#define MALLOC_TYPE long
#define MODULAR 7919

int main() {
    printf("Hello from C code\n");

    MALLOC_TYPE *addr = (MALLOC_TYPE *)malloc(MALLOC_SIZE);
    printf("malloc: %p\n", addr);

    MALLOC_TYPE start = 23;

    printf("Initializing...\n");

    for (MALLOC_TYPE *i = addr; i < addr + (MALLOC_SIZE / sizeof(MALLOC_TYPE));
         i += 4096 / sizeof(MALLOC_TYPE)) {
        *i = start;
        start = (start + 1) % MODULAR;
    }
    printf("Initializing... Done\n");
    start = 23;
    // int ret = madvise(addr, MALLOC_SIZE, MADV_DDC_PAGEOUT);
    // assert(ret == 0);
    printf("Testing...\n");
    for (MALLOC_TYPE *i = addr; i < addr + (MALLOC_SIZE / sizeof(MALLOC_TYPE));
         i += 4096 / sizeof(MALLOC_TYPE)) {
        if (*i != start) {
            printf("addr: %p %ld vs %ld\n", addr, *i, start);
        }
        assert(*i == start);
        start = (start + 1) % MODULAR;
    }
    printf("Testing... Done\n");

    return 0;
}
