#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef uint64_t (*ddc_eviction_mask_f)(uintptr_t vaddr);

void ddc_register_eviction_mask(ddc_eviction_mask_f f);

#ifdef __cplusplus
}
#endif