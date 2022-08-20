#include <ddc/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int *addr = (int *)mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_DDC, -1, 0);
    int *addr2 = (int *)mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS | MAP_DDC, -1, 0);
    printf("addr: %p, addr2: %p\n", addr, addr2);

    int test = 100;

    if (argc > 1) {
        test = atoi(argv[1]);
    }

    *addr = test;

    printf("*addr: %d\n", *addr);
    return 0;
}
