#ifndef QEMU_VIRTIO_UVERBS_H
#define QEMU_VIRTIO_UVERBS_H

#include "standard-headers/linux/virtio_ids.h"
#include "standard-headers/linux/virtio_config.h"

#define TYPE_VIRTIO_UVERBS "virtio-uverbs-device"
#define VIRTIO_UVERBS(obj) \
    OBJECT_CHECK(VirtIOUverbs, (obj), TYPE_VIRTIO_UVERBS)
#define VIRTIO_UVERBS_GET_PARENT_CLASS(obj) \
    OBJECT_GET_PARENT_CLASS(obj, TYPE_VIRTIO_UVERBS)


struct VirtIOUverbsConf
{
    char *host;  // uverbs name on host
};

typedef struct SysfsNode
{
    bool is_dir;
    char *path;
    GArray *entries;
} SysfsNode;

typedef struct VirtIOUverbs
{
    VirtIODevice parent_obj;

    VirtQueue *uverbsq;

    /* uverbs fd */
    int uverbs_fd;
    GArray *sysfs_path_list;

    VirtIOUverbsConf conf;
    VMChangeStateEntry *vmstate;
} VirtIOUverbs;

#endif
