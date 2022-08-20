#include "virtio-uverbs.hh"

#include <fs/vfs/vfs.h>
#include <sys/mman.h>

#include <array>
#include <cassert>
#include <ddc/alloc.hh>
#include <memory>
#include <osv/mmu-defs.hh>

#include "msr.hh"
#include "virtio-uverbs-trans.h"

enum {
    MLX5_IB_MMAP_CMD_SHIFT = 8,
    MLX5_IB_MMAP_CMD_MASK = 0xff,
};
enum mlx5_ib_mmap_cmd {
    MLX5_IB_MMAP_REGULAR_PAGE = 0,
    MLX5_IB_MMAP_WC_PAGE = 2,
    MLX5_IB_MMAP_NC_PAGE = 3,
    MLX5_IB_MMAP_MAP_DC_INFO_PAGE = 4,
    /* 5 is chosen in order to be compatible with old versions of libmlx5 */
    MLX5_IB_MMAP_CORE_CLOCK = 5,
    MLX5_IB_MMAP_ALLOC_WC = 6,
    MLX5_IB_MMAP_CLOCK_INFO = 7,
    MLX5_IB_MMAP_DEVICE_MEM = 8,
};
enum mlx5_ib_exp_mmap_cmd {
    MLX5_IB_MMAP_GET_CONTIGUOUS_PAGES = 1,
    MLX5_IB_EXP_MMAP_CORE_CLOCK = 0xFB,
    MLX5_IB_EXP_MMAP_GET_CONTIGUOUS_PAGES_CPU_NUMA = 0xFC,
    MLX5_IB_EXP_MMAP_GET_CONTIGUOUS_PAGES_DEV_NUMA = 0xFD,
    MLX5_IB_EXP_ALLOC_N_MMAP_WC = 0xFE,
    MLX5_IB_EXP_MMAP_CLOCK_INFO = 0xFF,
};

static int get_command(unsigned long offset) {
    int cmd = (offset >> MLX5_IB_MMAP_CMD_SHIFT) & MLX5_IB_MMAP_CMD_MASK;

    return (cmd == MLX5_IB_EXP_MMAP_CORE_CLOCK) ? MLX5_IB_MMAP_CORE_CLOCK : cmd;
}

// out_addr, out_bytes, in_bytes, success
// Note: out_bytes: device -> verbs
//       in bytes:  verbs -> device
//      this is opposite to virtio
namespace virtio {

int uverbs::_instance = 0;
static std::vector<uverbs *> uverbs_list;

std::vector<uverbs *> list_uverbs() { return uverbs_list; }
uverbs *get_uverbs_by_dev(std::string name) {
    for (auto &uverbs : uverbs_list) {
        if (uverbs->get_dev_name() == name) {
            return uverbs;
        }
    }
    return NULL;
}

uverbs *get_uverbs_by_ibdev(std::string name) {
    for (auto &uverbs : uverbs_list) {
        if (uverbs->get_ibdev_name() == name) {
            return uverbs;
        }
    }
    return NULL;
}

static int uverbs_write(struct device *dev, struct uio *uio, int ioflags) {
    auto *prv = reinterpret_cast<struct uverbs_priv *>(dev->private_data);
    return prv->drv->make_write_request(uio, ioflags);
}

static void *uverbs_mmap(struct device *dev, void *addr, size_t length,
                         unsigned flags, unsigned perm, off_t offset) {
    auto *prv = reinterpret_cast<struct uverbs_priv *>(dev->private_data);
    return prv->drv->make_mmap_request(addr, length, flags, perm, offset);
}

static int uverbs_munmap(struct device *dev, void *addr, size_t length) {
    auto *prv = reinterpret_cast<struct uverbs_priv *>(dev->private_data);
    return prv->drv->make_munmap_request(addr, length);
}

static int uverbs_ioctl(struct device *dev, unsigned long request, void *data) {
    printf("uverbs_ioctl\n");
    // abort();
    return ENODEV;
}

static struct devops uverbs_devops {
    no_open, no_close, no_read, uverbs_write, uverbs_ioctl, no_devctl,
        no_strategy, uverbs_mmap, uverbs_munmap,
};

struct driver uverbs_driver = {
    "uverbs",
    &uverbs_devops,
    sizeof(struct uverbs_priv),
};

uverbs::uverbs(virtio_device &virtio_dev) : virtio_driver(virtio_dev) {
    _driver_name = "virtio-uverbs";
    _id = _instance++;
    virtio_i("VIRTIO UVERBS INSTANCE %d", _id);
    // Steps 4, 5 & 6 - negotiate and confirm features
    setup_features();
    // Step 7 - generic init of virtqueues
    probe_virt_queues();

    // Step 8
    add_dev_status(VIRTIO_CONFIG_S_DRIVER_OK);

    struct uverbs_priv *prv;
    struct device *dev;
    _dev_name = "uverbs";
    _dev_name += std::to_string(_id);

    dev = device_create(&uverbs_driver, _dev_name.c_str(), D_CHR);
    prv = reinterpret_cast<struct uverbs_priv *>(dev->private_data);
    prv->drv = this;

    uverbs_list.push_back(this);

    _ibdev_name = get_info<4096>(VIRTIO_UVERBS_SYSFS_GET_INFO_IBDEV_NAME);

    debugf("virtio-uverbs: Add uverbs device instances %d as %s\n", _id,
           _dev_name.c_str());
}

std::string uverbs::get_ibdev_name() { return _ibdev_name; }

template <size_t max_info_size>
std::string uverbs::get_info(uint8_t info_type) {
    auto *queue = get_virt_queue(0);

    std::unique_ptr<struct uverbs_hdr> hdr(new struct uverbs_hdr);
    std::memset(hdr.get(), 0, sizeof(*hdr.get()));
    hdr->type = VIRTIO_UVERBS_SYSFS_GET_INFO;
    hdr->prot.get_info.info_type = info_type;

    std::unique_ptr<std::array<char, max_info_size>> buff(
        new std::array<char, max_info_size>);
    ;
    std::unique_ptr<struct uverbs_resp> resp(new struct uverbs_resp);

    queue->init_sg();
    queue->add_out_sg(hdr.get(), sizeof(*hdr.get()));
    queue->add_in_sg(buff->data(), sizeof(char) * max_info_size);
    queue->add_in_sg(resp.get(), sizeof(*resp.get()));

    queue->add_buf_wait(hdr.get());
    queue->kick();

    while (!queue->used_ring_not_empty())
        ;

    u32 ret;
    queue->get_buf_elem(&ret);
    queue->get_buf_finalize();

    assert(resp->status == VIRTIO_UVERBS_S_OK);

    return std::string(buff->data());
}

std::string uverbs::get_abi_version() {
    return get_info<4096>(VIRTIO_UVERBS_SYSFS_GET_INFO_ABI_VERSION);
}

int uverbs::make_readdir(u32 fsno, u32 ino, u64 offset, std::string &name,
                         bool *is_dir) {
    constexpr size_t max_name_len = 512;
    auto *queue = get_virt_queue(0);

    std::unique_ptr<struct uverbs_hdr> hdr(new struct uverbs_hdr);
    std::memset(hdr.get(), 0, sizeof(*hdr.get()));
    hdr->type = VIRTIO_UVERBS_SYSFS_READDIR;
    hdr->prot.readdir.fsno = fsno;
    hdr->prot.readdir.ino = ino;
    hdr->prot.readdir.offset = offset;
    std::unique_ptr<char[]> buff(new char[max_name_len]);
    ;
    std::unique_ptr<struct uverbs_resp> resp(new struct uverbs_resp);

    queue->init_sg();
    queue->add_out_sg(hdr.get(), sizeof(*hdr.get()));
    queue->add_in_sg(buff.get(), max_name_len);
    queue->add_in_sg(resp.get(), sizeof(*resp.get()));

    queue->add_buf_wait(hdr.get());
    queue->kick();

    while (!queue->used_ring_not_empty())
        ;

    u32 ret;
    queue->get_buf_elem(&ret);
    queue->get_buf_finalize();

    assert(resp->status == VIRTIO_UVERBS_S_OK);

    switch (resp->prot.readdir.error) {
        case VIRTIO_UVERBS_SYSFS_READDIR_OK:
            *is_dir = resp->prot.readdir.is_dir ? true : false;
            name = buff.get();
            return 0;
        case VIRTIO_UVERBS_SYSFS_READDIR_NOENT:
            return ENOENT;
        case VIRTIO_UVERBS_SYSFS_READDIR_INVAL:
            return EINVAL;
        default:
            return EINVAL;
    }
}

int uverbs::make_lookup(u32 fsno, u32 ino, std::string &name, u32 *lookup_ino,
                        bool *is_dir) {
    auto *queue = get_virt_queue(0);

    std::unique_ptr<struct uverbs_hdr> hdr(new struct uverbs_hdr);
    std::memset(hdr.get(), 0, sizeof(*hdr.get()));
    hdr->type = VIRTIO_UVERBS_SYSFS_LOOKUP;
    hdr->prot.lookup.fsno = fsno;
    hdr->prot.lookup.ino = ino;

    size_t buff_len = name.length() + 1;
    std::unique_ptr<char[]> buff(new char[buff_len]);

    strncpy(buff.get(), name.c_str(), buff_len);

    std::unique_ptr<struct uverbs_resp> resp(new struct uverbs_resp);

    queue->init_sg();
    queue->add_out_sg(hdr.get(), sizeof(*hdr.get()));
    queue->add_out_sg(buff.get(), buff_len);
    queue->add_in_sg(resp.get(), sizeof(*resp.get()));

    queue->add_buf_wait(hdr.get());
    queue->kick();

    while (!queue->used_ring_not_empty())
        ;

    u32 ret;
    queue->get_buf_elem(&ret);
    queue->get_buf_finalize();

    assert(resp->status == VIRTIO_UVERBS_S_OK);

    switch (resp->prot.lookup.error) {
        case VIRTIO_UVERBS_SYSFS_LOOKUP_OK:
            *is_dir = resp->prot.lookup.is_dir ? true : false;
            *lookup_ino = resp->prot.lookup.ino;
            return 0;
        case VIRTIO_UVERBS_SYSFS_LOOKUP_NOENT:
            return ENOENT;
        default:
            return EINVAL;
    }
}

// Return bytes_read or -error
// buff must be in heap
int uverbs::make_read_sys(u32 fsno, u32 ino, char *buff, size_t len,
                          size_t offset) {
    auto *queue = get_virt_queue(0);

    std::unique_ptr<struct uverbs_hdr> hdr(new struct uverbs_hdr);
    std::memset(hdr.get(), 0, sizeof(*hdr.get()));
    hdr->type = VIRTIO_UVERBS_SYSFS_READ;
    hdr->prot.read.fsno = fsno;
    hdr->prot.read.ino = ino;
    hdr->prot.read.offset = offset;

    std::unique_ptr<struct uverbs_resp> resp(new struct uverbs_resp);

    queue->init_sg();
    queue->add_out_sg(hdr.get(), sizeof(*hdr.get()));
    queue->add_in_sg(buff, len);
    queue->add_in_sg(resp.get(), sizeof(*resp.get()));

    queue->add_buf_wait(hdr.get());
    queue->kick();
    while (!queue->used_ring_not_empty())
        ;

    u32 ret;
    queue->get_buf_elem(&ret);
    queue->get_buf_finalize();

    assert(resp->status == VIRTIO_UVERBS_S_OK);

    switch (resp->prot.read.error) {
        case VIRTIO_UVERBS_SYSFS_READ_OK:
            return ret;
        case VIRTIO_UVERBS_SYSFS_READ_ISDIR:
            return -EISDIR;
        default:
            return -EINVAL;
    }
}

int uverbs::make_write_request(struct uio *uio, int ioflags) {
    if (uio->uio_iovcnt != 1) {
        printf("uio->uio_iovcnt IS NOT 1\n");
        return -1;
    }

    std::unique_ptr<struct uverbs_hdr> hdr(new struct uverbs_hdr);
    std::memset(hdr.get(), 0, sizeof(*hdr.get()));
    hdr->type = VIRTIO_UVERBS_DEV_WRITE;

    size_t bytes_to_write = uio->uio_iov[0].iov_len;

    std::unique_ptr<char[]> buff(new char[bytes_to_write]);
    memcpy(buff.get(), uio->uio_iov[0].iov_base, bytes_to_write);

    void *uverbs_out_addr;
    size_t uverbs_out_bytes;
    size_t uverbs_in_bytes;
    int err = virtio_uverbs_translate_ofed(buff.get(), &uverbs_out_addr,
                                           &uverbs_out_bytes, &uverbs_in_bytes);

    if (err) return -1;

    if (uverbs_in_bytes != bytes_to_write) return -1;

    std::unique_ptr<char[]> response(
        uverbs_out_bytes > 0 ? new char[uverbs_out_bytes] : nullptr);

    std::unique_ptr<struct uverbs_resp> resp(new struct uverbs_resp);
    auto *queue = get_virt_queue(0);
    queue->init_sg();
    queue->add_out_sg(hdr.get(), sizeof(*hdr.get()));
    queue->add_out_sg(buff.get(), bytes_to_write);
    if (response.get() != nullptr) {
        queue->add_in_sg(response.get(), uverbs_out_bytes);
    }
    queue->add_in_sg(resp.get(), sizeof(*resp.get()));

    queue->add_buf_wait(hdr.get());
    queue->kick();

    while (!queue->used_ring_not_empty())
        ;

    u32 len;
    queue->get_buf_elem(&len);
    queue->get_buf_finalize();

    assert(resp->status == VIRTIO_UVERBS_S_OK);
    if (response.get() != nullptr) {
        memcpy(uverbs_out_addr, response.get(), uverbs_out_bytes);
    }

    if (uio->uio_resid < len) {
        abort("uio->uio_resid < len!!!!!!!!!!!!!!!\n");
    }
    uio->uio_offset += len;
    uio->uio_resid -= len;

    return 0;
}

void *uverbs::make_mmap_request(void *addr, size_t length, unsigned flags,
                                unsigned perm, off_t offset) {
    unsigned long command = get_command(offset >> mmu::page_size_shift);

    std::unique_ptr<struct uverbs_hdr> hdr(new struct uverbs_hdr);
    std::memset(hdr.get(), 0, sizeof(*hdr.get()));
    hdr->type = VIRTIO_UVERBS_DEV_MMAP;

    hdr->prot.mmap.prot = 0;

    hdr->prot.mmap.prot |= perm & mmu::perm_read ? PROT_READ : 0;
    hdr->prot.mmap.prot |= perm & mmu::perm_write ? PROT_WRITE : 0;
    hdr->prot.mmap.prot |= perm & mmu::perm_exec ? PROT_EXEC : 0;

    bool fixed = flags & mmu::mmap_fixed;
    bool shared = flags & mmu::mmap_shared;

    if (addr || fixed) {
        abort("uverbs mmap fail\n");
    }

    hdr->prot.mmap.flags = 0;

    hdr->prot.mmap.flags |= fixed ? MAP_FIXED : 0;
    hdr->prot.mmap.flags |= shared ? MAP_SHARED : MAP_PRIVATE;

    hdr->prot.mmap.offset = offset;

    addr = ddc::alloc::pre_alloc_buffer(length);  // free on unmap

    if (command == MLX5_IB_MMAP_REGULAR_PAGE) {
        // Make write combine
        mmu::linear_map(addr, mmu::virt_to_phys(addr), length, mmu::page_size,
                        mmu::mattr::writecombine);
    }

    std::unique_ptr<struct uverbs_resp> resp(new struct uverbs_resp);

    auto *queue = get_virt_queue(0);
    queue->init_sg();
    queue->add_out_sg(hdr.get(), sizeof(*hdr.get()));
    queue->add_in_sg(addr, length);
    queue->add_in_sg(resp.get(), sizeof(*resp.get()));
    queue->add_buf_wait(hdr.get());
    queue->kick();

    while (!queue->used_ring_not_empty())
        ;

    u32 len;
    queue->get_buf_elem(&len);
    queue->get_buf_finalize();

    assert(resp->status == VIRTIO_UVERBS_S_OK);

    if (resp->prot.mmap.error == VIRTIO_UVERBS_DEV_MMAP_FAIL) {
        return NULL;
    }
    return addr;
}
int uverbs::make_munmap_request(void *addr, size_t length) {
    std::unique_ptr<struct uverbs_hdr> hdr(new struct uverbs_hdr);
    std::memset(hdr.get(), 0, sizeof(*hdr.get()));
    hdr->type = VIRTIO_UVERBS_DEV_MUNMAP;

    std::unique_ptr<struct uverbs_resp> resp(new struct uverbs_resp);

    auto *queue = get_virt_queue(0);
    queue->init_sg();
    queue->add_out_sg(hdr.get(), sizeof(*hdr.get()));
    queue->add_out_sg(addr, length);
    queue->add_in_sg(resp.get(), sizeof(*resp.get()));
    queue->add_buf_wait(hdr.get());
    queue->kick();

    while (!queue->used_ring_not_empty())
        ;

    u32 len;
    queue->get_buf_elem(&len);
    queue->get_buf_finalize();

    assert(resp->status == VIRTIO_UVERBS_S_OK);

    if (resp->prot.mmap.error == VIRTIO_UVERBS_DEV_MUNMAP_FAIL) {
        return -1;
    }
    return 0;
}

uverbs::~uverbs() {}

hw_driver *uverbs::probe(hw_device *dev) {
    return virtio::probe<uverbs, VIRTIO_ID_UVERBS>(dev);
}

}  // namespace virtio