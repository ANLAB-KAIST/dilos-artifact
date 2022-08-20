#ifndef VIRTIO_UVERBS_TRANS_QEMU_H
#define VIRTIO_UVERBS_TRANS_QEMU_H

#define LIBUVERBS_UPDATE_ATTR(object_ptr, attr) \
    object_ptr->attr = (uintptr_t)gpa2hva((hwaddr)object_ptr->attr)

static inline void *gpa2hva(hwaddr addr) {
    MemoryRegionSection mrs = memory_region_find(get_system_memory(), addr, 1);

    if (!mrs.mr) {
        printf("No memory is mapped at address 0x%lx\n", addr);
        return NULL;
    }

    if (!memory_region_is_ram(mrs.mr) && !memory_region_is_romd(mrs.mr)) {
        printf("Memory at address 0x%lx is not RAM\n", addr);
        memory_region_unref(mrs.mr);
        return NULL;
    }

    void *hva = qemu_map_ram_ptr(mrs.mr->ram_block, mrs.offset_within_region);
    memory_region_unref(mrs.mr);
    return hva;
}

#endif