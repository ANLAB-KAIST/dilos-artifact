#include <osv/device.h>
#include <osv/mount.h>

#include <ddc/dev_vma.hh>
#include <fs/fs.hh>

#include "fs/vfs/vfs.h"

namespace mmu {

dev_vma::dev_vma(addr_range range, unsigned perm, unsigned flags, fileref file,
                 f_offset offset, struct device *dev)
    : file_vma(range, perm, flags, file, offset, nullptr), _dev(dev) {}

void dev_vma::split(uintptr_t edge) {
    assert(edge == start() || edge == end());
}

error dev_vma::sync(uintptr_t start, uintptr_t end) { return no_error(); }
int dev_vma::do_munmap(uintptr_t start, uintptr_t end) {
    struct driver *driver = _dev->driver;

    void *addr = reinterpret_cast<void *>(start);
    size_t length = end - start;

    return driver->devops->munmap(_dev, addr, length);
}

std::unique_ptr<dev_vma> map_dev_mmap(file *file, addr_range range,
                                      unsigned flags, unsigned perm,
                                      off_t offset) {
    struct vnode *vp = file->f_dentry->d_vnode;
    struct mount *mount = vp->v_mount;

    if (strncmp(mount->m_path, "/dev", 4) != 0)
        return std::unique_ptr<dev_vma>();

    struct device *dev = (struct device *)vp->v_data;
    struct driver *driver = dev->driver;

    if (driver->devops->mmap == no_mmap) return std::unique_ptr<dev_vma>();

    if (range.start() != 0) {
        throw make_error(ENOMEM);
    }

    size_t len = range.end() - range.start();
    void *mmaped_addr =
        driver->devops->mmap(dev, NULL, len, flags, perm, offset);

    if (mmaped_addr == NULL) return std::unique_ptr<dev_vma>();

    uintptr_t mmaped_addr_u = reinterpret_cast<uintptr_t>(mmaped_addr);
    addr_range new_range(mmaped_addr_u, mmaped_addr_u + len);

    assert(!(flags & mmu::mmap_populate));
    flags |= mmu::mmap_fixed;
    return std::unique_ptr<dev_vma>(
        new dev_vma(new_range, perm, flags, file, offset, dev));
}

}  // namespace mmu
