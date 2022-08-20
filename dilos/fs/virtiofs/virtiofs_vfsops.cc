/*
 * Copyright (C) 2020 Waldemar Kozaczuk
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <sys/types.h>
#include <osv/device.h>
#include <osv/debug.h>
#include <iomanip>
#include <iostream>
#include "virtiofs.hh"
#include "virtiofs_i.hh"

static std::atomic<uint64_t> fuse_unique_id(1);

int fuse_req_send_and_receive_reply(fuse_strategy* strategy, uint32_t opcode,
    uint64_t nodeid, void* input_args_data, size_t input_args_size,
    void* output_args_data, size_t output_args_size)
{
    std::unique_ptr<fuse_request> req {new (std::nothrow) fuse_request()};
    if (!req) {
        return ENOMEM;
    }
    req->in_header.len = sizeof(req->in_header) + input_args_size;
    req->in_header.opcode = opcode;
    req->in_header.unique = fuse_unique_id.fetch_add(1,
        std::memory_order_relaxed);
    req->in_header.nodeid = nodeid;

    req->input_args_data = input_args_data;
    req->input_args_size = input_args_size;

    req->output_args_data = output_args_data;
    req->output_args_size = output_args_size;

    assert(strategy->drv);
    strategy->make_request(strategy->drv, req.get());
    fuse_req_wait(req.get());

    int error = -req->out_header.error;

    return error;
}

void virtiofs_set_vnode(struct vnode* vnode, struct virtiofs_inode* inode)
{
    if (!vnode || !inode) {
        return;
    }

    vnode->v_data = inode;
    vnode->v_ino = inode->nodeid;

    // Set type
    if (S_ISDIR(inode->attr.mode)) {
        vnode->v_type = VDIR;
    } else if (S_ISREG(inode->attr.mode)) {
        vnode->v_type = VREG;
    } else if (S_ISLNK(inode->attr.mode)) {
        vnode->v_type = VLNK;
    }

    vnode->v_mode = 0555;
    vnode->v_size = inode->attr.size;
}

static int virtiofs_mount(struct mount* mp, const char* dev, int flags,
    const void* data)
{
    struct device* device;

    int error = device_open(dev + strlen("/dev/"), DO_RDWR, &device);
    if (error) {
        kprintf("[virtiofs] Error opening device!\n");
        return error;
    }

    std::unique_ptr<fuse_init_in> in_args {new (std::nothrow) fuse_init_in()};
    std::unique_ptr<fuse_init_out> out_args {new (std::nothrow) fuse_init_out};
    if (!in_args || !out_args) {
        return ENOMEM;
    }
    in_args->major = FUSE_KERNEL_VERSION;
    in_args->minor = FUSE_KERNEL_MINOR_VERSION;
    in_args->max_readahead = PAGE_SIZE;
    in_args->flags = 0; // TODO: Verify that we need not set any flag

    auto* strategy = static_cast<fuse_strategy*>(device->private_data);
    error = fuse_req_send_and_receive_reply(strategy, FUSE_INIT, FUSE_ROOT_ID,
        in_args.get(), sizeof(*in_args), out_args.get(), sizeof(*out_args));
    if (error) {
        kprintf("[virtiofs] Failed to initialize fuse filesystem!\n");
        return error;
    }
    // TODO: Handle version negotiation

    virtiofs_debug("Initialized fuse filesystem with version major: %d, "
                   "minor: %d\n", out_args->major, out_args->minor);

    auto* root_node {new (std::nothrow) virtiofs_inode()};
    if (!root_node) {
        return ENOMEM;
    }
    root_node->nodeid = FUSE_ROOT_ID;
    root_node->attr.mode = S_IFDIR;

    virtiofs_set_vnode(mp->m_root->d_vnode, root_node);

    mp->m_data = strategy;
    mp->m_dev = device;

    return 0;
}

static int virtiofs_sync(struct mount* mp)
{
    return 0;
}

static int virtiofs_statfs(struct mount* mp, struct statfs* statp)
{
    // TODO: Call FUSE_STATFS

    // Read only. 0 blocks free
    statp->f_bfree = 0;
    statp->f_bavail = 0;

    statp->f_ffree = 0;

    return 0;
}

static int virtiofs_unmount(struct mount* mp, int flags)
{
    struct device* dev = mp->m_dev;
    return device_close(dev);
}

#define virtiofs_vget ((vfsop_vget_t)vfs_nullop)

struct vfsops virtiofs_vfsops = {
    virtiofs_mount,		/* mount */
    virtiofs_unmount,	/* unmount */
    virtiofs_sync,		/* sync */
    virtiofs_vget,      /* vget */
    virtiofs_statfs,	/* statfs */
    &virtiofs_vnops	    /* vnops */
};
