#pragma once

#include <assert.h>
#include <osv/types.h>
//
#include <bsd/porting/mmu.h>

#define LIBUVERBS_UPDATE_ATTR(object_ptr, attr) \
    object_ptr->attr = (uintptr_t)virt_to_phys((void *)object_ptr->attr)

#ifdef __cplusplus
extern "C" {
#endif

int virtio_uverbs_translate_ofed(char *buf, void **out_addr,
                                 uint64_t *out_bytes, uint64_t *in_bytes);

#ifdef __cplusplus
}
#endif