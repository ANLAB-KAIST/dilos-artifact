#include <osv/dentry.h>
#include <osv/file.h>
#include <osv/mount.h>
#include <osv/prex.h>
#include <osv/vnode.h>

#include <cassert>
#include <cerrno>
#include <ddc/ibsysfs.hh>

#include "fs/pseudofs/pseudofs.hh"
#include "virtio-uverbs.hh"

/*
 * Mount a file system.
 */

// mount flags: fsno
// data: uverbs*
static int ibsysfs_op_mount(struct mount *mp, const char *dev, int flags,
                            const void *data) {
    assert(mp);

    // virtio::uverbs *uverbs_dev = (virtio::uverbs *)data;

    mp->m_data = (void *)data;
    return 0;
}

/*
 * Unmount a file system.
 *
 * Note: There is no ibsysfs_unmount in ibsysfslib.
 *
 */
static int ibsysfs_op_unmount(struct mount *mp, int flags) {
    assert(mp);

    // Make sure nothing is used under this mount point.
    if (mp->m_count > 1) {
        return EBUSY;
    }

    return 0;
}

// For the following let's rely on operations on individual files
#define ibsysfs_op_sync ((vfsop_sync_t)vfs_nullop)
#define ibsysfs_op_vget ((vfsop_vget_t)vfs_nullop)
#define ibsysfs_op_statfs ((vfsop_statfs_t)vfs_nullop)

static std::string abi_version("");
static std::string get_abi_version() { return abi_version; }

int ibsysfs_prepare(std::shared_ptr<pseudofs::pseudo_dir_node> infiniband,
                    std::shared_ptr<pseudofs::pseudo_dir_node> infiniband_verbs,
                    uint64_t &inode_count) {
    auto uverbs_list = virtio::list_uverbs();
    if (uverbs_list.size() == 0) {
        return 0;
    }

    std::string abi_version_string("");

    for (auto &uverbs : uverbs_list) {
        auto abi_version_this = uverbs->get_abi_version();

        if (abi_version_string != "" &&
            abi_version_string != abi_version_this) {
            debug_early("ABI version conflict\n");
            return 1;
        }
        abi_version_string = abi_version_this;
    }

    abi_version = abi_version_string;
    infiniband_verbs->add("abi_version", inode_count++, get_abi_version);

    for (auto &uverbs : uverbs_list) {
        auto uverbs_dir =
            std::make_shared<pseudofs::pseudo_dir_node>(inode_count++);
        infiniband_verbs->add(uverbs->get_dev_name().c_str(), uverbs_dir);

        auto ibdev_dir =
            std::make_shared<pseudofs::pseudo_dir_node>(inode_count++);
        infiniband->add(uverbs->get_ibdev_name().c_str(), ibdev_dir);
    }

    return 0;
}

int ibsysfs_mount() {
    auto uverbs_list = virtio::list_uverbs();
    int ret;
    for (auto &uverbs : uverbs_list) {
        std::string mp("/sys/class/infiniband_verbs/");
        auto dev_name = uverbs->get_dev_name();
        mp += dev_name;

        ret = sys_mount(dev_name.c_str(), mp.c_str(), "ibsysfs", 0,
                        (void *)uverbs);
        if (ret) return ret;
        auto ibdev_name = uverbs->get_ibdev_name();
        std::string mp2("/sys/class/infiniband/");
        mp2 += ibdev_name;
        ret = sys_mount(ibdev_name.c_str(), mp2.c_str(), "ibsysfs", 1,
                        (void *)uverbs);
        if (ret) return ret;
    }

    return 0;
}

static mutex_t ibsysfs_mutex;

static int ibsysfs_op_read(vnode *vp, file *fp, uio *uio, int ioflags) {
    if (uio->uio_offset < 0) return EINVAL;
    if (uio->uio_iovcnt != 1) return EINVAL;

    std::lock_guard<mutex_t> lock(ibsysfs_mutex);

    size_t read_offset = uio->uio_offset;
    size_t bytes_to_read = uio->uio_resid;
    // Use heap for buffer
    std::unique_ptr<char[]> buff(new char[bytes_to_read]);

    u32 ino = reinterpret_cast<u64>(vp->v_data);
    virtio::uverbs *dev = static_cast<virtio::uverbs *>(vp->v_mount->m_data);
    u32 fsno = vp->v_mount->m_flags;

    int ret =
        dev->make_read_sys(fsno, ino, buff.get(), bytes_to_read, read_offset);
    if (ret <= 0) {
        return -ret;
    }

    return uiomove(buff.get(), ret, uio);
}
static int ibsysfs_op_readdir(vnode *vp, file *fp, dirent *dir) {
    std::lock_guard<mutex_t> lock(ibsysfs_mutex);
    switch (fp->f_offset) {
        case 0: {
            dir->d_type = DT_DIR;
            if (vfs_dname_copy((char *)&dir->d_name, ".",
                               sizeof(dir->d_name))) {
                return EINVAL;
            }
            break;
        }
        case 1: {
            dir->d_type = DT_DIR;
            if (vfs_dname_copy((char *)&dir->d_name, "..",
                               sizeof(dir->d_name))) {
                return EINVAL;
            }
            break;
        }
        default: {
            // Return 할 수 있는 것 들
            // ENOENT: 내용이 없으면
            // EINVAL: 기타

            // 세팅해야 할 값
            // dir->d_type
            // dir->d_name
            // 아래는 리턴 전 마지막에 세팅
            // fp->f_offset
            // dir->d_fileno

            u32 ino = reinterpret_cast<u64>(vp->v_data);
            std::string d_name;

            bool is_dir;
            virtio::uverbs *dev =
                static_cast<virtio::uverbs *>(vp->v_mount->m_data);

            u32 fsno = vp->v_mount->m_flags;

            if (dev == NULL) {
                return ENOENT;
            }

            int ret =
                dev->make_readdir(fsno, ino, fp->f_offset, d_name, &is_dir);
            if (ret != 0) {
                return ret;
            }

            if (is_dir) {
                dir->d_type = DT_DIR;
            } else {
                dir->d_type = DT_REG;
            }

            if (vfs_dname_copy((char *)&dir->d_name, d_name.c_str(), 256)) {
                return EINVAL;
            }
        }
    }

    dir->d_fileno = fp->f_offset;
    fp->f_offset++;

    return 0;
}

static int ibsysfs_op_lookup(vnode *dvp, char *name, vnode **vpp) {
    std::lock_guard<mutex_t> lock(ibsysfs_mutex);

    // ENOENT: dir이 아니거나 이름이 업으면, lookup이 아니면
    *vpp = nullptr;
    if (!*name) {
        return ENOENT;
    }

    u32 ino = reinterpret_cast<u64>(dvp->v_data);
    virtio::uverbs *dev = static_cast<virtio::uverbs *>(dvp->v_mount->m_data);

    std::string name_s(name);

    u32 fsno = dvp->v_mount->m_flags;

    u32 lookup_ino;
    bool is_dir;
    auto ret = dev->make_lookup(fsno, ino, name_s, &lookup_ino, &is_dir);

    if (ret != 0) {
        return ret;
    }

    vnode *vp;
    if (vget(dvp->v_mount, lookup_ino, &vp)) {
        // found in cache
        *vpp = vp;
        return 0;
    }
    if (!vp) {
        return ENOMEM;
    }

    vp->v_data = (void *)(u64)lookup_ino;

    if (is_dir) {
        vp->v_type = VDIR;
        vp->v_mode = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
    } else {
        vp->v_type = VREG;
        vp->v_mode = S_IRUSR | S_IRGRP | S_IROTH;
    }
    vp->v_size = 0;

    *vpp = vp;

    return 0;
}
static int ibsysfs_op_getattr(vnode *vp, vattr *attr) {
    attr->va_nodeid = vp->v_ino;
    attr->va_size = vp->v_size;
    return 0;
}

int ibsysfs_init(void) { return 0; }

#define ibsysfs_op_open ((vnop_open_t)vop_nullop)
#define ibsysfs_op_close ((vnop_close_t)vop_nullop)
#define ibsysfs_op_write ((vnop_write_t)vop_einval)
#define ibsysfs_op_seek ((vnop_seek_t)vop_nullop)
#define ibsysfs_op_ioctl ((vnop_ioctl_t)vop_einval)
#define ibsysfs_op_fsync ((vnop_fsync_t)vop_nullop)
#define ibsysfs_op_create ((vnop_create_t)vop_einval)
#define ibsysfs_op_remove ((vnop_remove_t)vop_einval)
#define ibsysfs_op_rename ((vnop_rename_t)vop_einval)
#define ibsysfs_op_mkdir ((vnop_mkdir_t)vop_einval)
#define ibsysfs_op_rmdir ((vnop_rmdir_t)vop_einval)
#define ibsysfs_op_setattr ((vnop_setattr_t)vop_eperm)
#define ibsysfs_op_inactive ((vnop_inactive_t)vop_nullop)
#define ibsysfs_op_truncate ((vnop_truncate_t)vop_nullop)
#define ibsysfs_op_link ((vnop_link_t)vop_eperm)
#define ibsysfs_op_fallocate ((vnop_fallocate_t)vop_nullop)
#define ibsysfs_op_readlink ((vnop_readlink_t)vop_nullop)
#define ibsysfs_op_symlink ((vnop_symlink_t)vop_nullop)

/*
 * vnode operations
 */
struct vnops ibsysfs_vnops = {
    ibsysfs_op_open,        /* open */
    ibsysfs_op_close,       /* close */
    ibsysfs_op_read,        /* read */
    ibsysfs_op_write,       /* write */
    ibsysfs_op_seek,        /* seek */
    ibsysfs_op_ioctl,       /* ioctl */
    ibsysfs_op_fsync,       /* fsync */
    ibsysfs_op_readdir,     /* readdir */
    ibsysfs_op_lookup,      /* lookup */
    ibsysfs_op_create,      /* create */
    ibsysfs_op_remove,      /* remove */
    ibsysfs_op_rename,      /* remame */
    ibsysfs_op_mkdir,       /* mkdir */
    ibsysfs_op_rmdir,       /* rmdir */
    ibsysfs_op_getattr,     /* getattr */
    ibsysfs_op_setattr,     /* setattr */
    ibsysfs_op_inactive,    /* inactive */
    ibsysfs_op_truncate,    /* truncate */
    ibsysfs_op_link,        /* link */
    (vnop_cache_t) nullptr, /* arc */
    ibsysfs_op_fallocate,   /* fallocate */
    ibsysfs_op_readlink,    /* read link */
    ibsysfs_op_symlink,     /* symbolic link */
};

#define ibsysfs_op_sync ((vfsop_sync_t)vfs_nullop)
#define ibsysfs_op_vget ((vfsop_vget_t)vfs_nullop)
#define ibsysfs_op_statfs ((vfsop_statfs_t)vfs_nullop)

struct vfsops ibsysfs_vfsops = {
    ibsysfs_op_mount,   /* mount */
    ibsysfs_op_unmount, /* umount */
    ibsysfs_op_sync,    /* sync */
    ibsysfs_op_vget,    /* vget */
    ibsysfs_op_statfs,  /* statfs */
    &ibsysfs_vnops,     /* vnops */
};
