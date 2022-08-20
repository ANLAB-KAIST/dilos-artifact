#ifndef VIRTIO_UVERBS_TRANS_PROT_H
#define VIRTIO_UVERBS_TRANS_PROT_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#define VIRTIO_UVERBS_SYSFS_GET_INFO 0
#define VIRTIO_UVERBS_SYSFS_READDIR 1
#define VIRTIO_UVERBS_SYSFS_LOOKUP 2
#define VIRTIO_UVERBS_SYSFS_READ 3
#define VIRTIO_UVERBS_DEV_WRITE 4
#define VIRTIO_UVERBS_DEV_MMAP 5
#define VIRTIO_UVERBS_DEV_MUNMAP 6

#define VIRTIO_UVERBS_SYSFS_GET_INFO_ABI_VERSION 0
#define VIRTIO_UVERBS_SYSFS_GET_INFO_IBDEV_NAME 1

#define VIRTIO_UVERBS_SYSFS_READDIR_OK 0
#define VIRTIO_UVERBS_SYSFS_READDIR_NOENT 1
#define VIRTIO_UVERBS_SYSFS_READDIR_INVAL 2

#define VIRTIO_UVERBS_SYSFS_LOOKUP_OK 0
#define VIRTIO_UVERBS_SYSFS_LOOKUP_NOENT 1

#define VIRTIO_UVERBS_SYSFS_READ_OK 0
#define VIRTIO_UVERBS_SYSFS_READ_ISDIR 1

#define VIRTIO_UVERBS_DEV_MMAP_OK 0
#define VIRTIO_UVERBS_DEV_MMAP_FAIL 1

#define VIRTIO_UVERBS_DEV_MUNMAP_OK 0
#define VIRTIO_UVERBS_DEV_MUNMAP_FAIL 1

#define VIRTIO_UVERBS_S_OK 0
#define VIRTIO_UVERBS_S_ERR 1
#define VIRTIO_UVERBS_S_UNSUPP 2

struct uverbs_hdr {
    uint32_t type;
    union {
        struct {
            uint8_t info_type;
        } get_info;
        struct {
            uint32_t fsno;
            uint32_t ino;
            uint64_t offset;
        } readdir;
        struct {
            uint32_t fsno;
            uint32_t ino;

        } lookup;
        struct {
            uint32_t fsno;
            uint32_t ino;
            uint64_t offset;
        } read;

        struct {
            uint64_t offset;  // NOTE: actually it is not needed.
        } write;

        struct {
            int32_t prot;
            int32_t flags;
            uint32_t offset;
        } mmap;

        struct {
        } munmp;
    } prot;
} __attribute__((packed));

struct uverbs_resp {
    uint32_t type;
    uint32_t status;
    union {
        struct {
        } abi_version;
        struct {
            uint8_t error;
            uint8_t is_dir;
        } readdir;
        struct {
            uint8_t error;
            uint8_t is_dir;
            uint32_t ino;

        } lookup;
        struct {
            uint8_t error;

        } read;

        struct {
        } write;

        struct {
            uint8_t error;
        } mmap;

        struct {
            uint8_t error;
        } munmp;
    } prot;
} __attribute__((packed));

#ifdef __cplusplus
}
#endif
#endif