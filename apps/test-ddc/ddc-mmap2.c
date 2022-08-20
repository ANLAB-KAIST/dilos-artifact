#include <assert.h>
#include <ddc/mman.h>
#include <stdio.h>

#define MMAP_SIZE (1ULL << 20)
#define MMAP_TYPE long
#define MODULAR 7919

int main() {
    printf("Hello from C code\n");

    MMAP_TYPE *addr =
        (MMAP_TYPE *)mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS | MAP_DDC, -1, 0);
    printf("mmap: %p\n", addr);

    MMAP_TYPE start = 23;

    printf("Initializing...\n");

    for (MMAP_TYPE *i = addr; i < addr + (MMAP_SIZE / sizeof(MMAP_TYPE));
         i += 4096 / sizeof(MMAP_TYPE)) {
        *i = start;
        start = (start + 1) % MODULAR;
    }
    printf("Initializing... Done\n");
    start = 23;
    int ret = madvise(addr, MMAP_SIZE, MADV_DDC_PAGEOUT);
    assert(ret == 0);
    printf("Testing...\n");
    for (MMAP_TYPE *i = addr; i < addr + (MMAP_SIZE / sizeof(MMAP_TYPE));
         i += 4096 / sizeof(MMAP_TYPE)) {
        assert(*i == start);
        start = (start + 1) % MODULAR;
    }
    printf("Testing... Done\n");

    return 0;
}
