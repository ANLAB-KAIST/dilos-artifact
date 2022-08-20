#ifndef DEV_MMU_HH
#define DEV_MMU_HH

#include <osv/device.h>

#include <osv/mmu.hh>
#include <osv/vfs_file.hh>

namespace mmu {

class dev_vma : public file_vma {
   public:
    dev_vma(addr_range range, unsigned perm, unsigned flags, fileref file,
            f_offset offset, struct device *dev);
    virtual void split(uintptr_t edge) override;
    virtual error sync(uintptr_t start, uintptr_t end) override;
    int do_munmap(uintptr_t start, uintptr_t end);

   private:
    struct device *_dev;
};

std::unique_ptr<dev_vma> map_dev_mmap(file *file, addr_range range,
                                      unsigned flags, unsigned perm,
                                      off_t offset);

}  // namespace mmu

#endif