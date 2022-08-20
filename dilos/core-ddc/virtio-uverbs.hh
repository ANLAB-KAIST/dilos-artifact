#pragma once

#include <osv/device.h>
#include <osv/mutex.h>
#include <virtio-uverbs/prot.h>

#include <array>

#include "drivers/device.hh"
#include "drivers/virtio.hh"

namespace virtio {

class uverbs : public virtio_driver {
   public:
    explicit uverbs(virtio_device &dev);
    virtual ~uverbs();

    virtual std::string get_name() const { return _driver_name; }
    virtual std::string get_dev_name() const { return _dev_name; }
    std::string get_ibdev_name();

    std::string get_abi_version();
    int make_read_request(struct uio *uio, int ioflags);
    int make_readdir(u32 fsno, u32 ino, u64 offset, std::string &name,
                     bool *is_dir);
    int make_lookup(u32 fsno, u32 ino, std::string &name, u32 *lookup_ino,
                    bool *is_dir);
    int make_read_sys(u32 fsno, u32 ino, char *buff, size_t len, size_t offset);
    int make_write_request(struct uio *uio, int ioflags);
    void *make_mmap_request(void *addr, size_t length, unsigned flags,
                            unsigned perm, off_t offset);
    int make_munmap_request(void *addr, size_t length);

    static hw_driver *probe(hw_device *dev);

   private:
    std::string _driver_name;
    std::string _dev_name;
    std::string _ibdev_name;
    static int _instance;
    int _id;

    template <size_t max_info_size>
    std::string get_info(uint8_t info_type);
};

// MUST NOT FREE uverbs*
std::vector<uverbs *> list_uverbs();
// MUST NOT FREE uverbs*
uverbs *get_uverbs_by_dev(std::string name);
uverbs *get_uverbs_by_ibdev(std::string name);

struct uverbs_priv {
    uverbs *drv;
};

}  // namespace virtio
