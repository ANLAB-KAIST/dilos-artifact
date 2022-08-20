#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern uint64_t ddc_eviction_mask_test(uintptr_t vaddr);

int main() {
    void *addr[10];

    for (int i = 0; i < 10; ++i) {
        addr[i] = malloc(128);
    }

    free(addr[3]);
    free(addr[8]);
    printf("addr: %p\n", addr);
    uint64_t ret = ddc_eviction_mask_test((((uintptr_t)*addr) >> 12) << 12);
    printf("ret: %lx\n", ret);

    return 0;
}