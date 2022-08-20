
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/iov.h"
#include "qemu/module.h"
#include "hw/qdev-properties.h"
#include "hw/virtio/virtio.h"
#include "hw/virtio/virtio-uverbs.h"
#include "qom/object_interfaces.h"

#include "exec/address-spaces.h"
#include "sysemu/cpus.h"
#include "sysemu/runstate.h"

#include <infiniband/kern-abi.h>
#include <rdma/mlx5-abi.h>

#include "../../../virtio-uverbs-trans/virtio-uverbs/prot.h"
#include "../../../virtio-uverbs-trans/virtio-uverbs/qemu.h"
#include "../../../virtio-uverbs-trans/virtio-uverbs/ibverbs.h"
#include "../../../virtio-uverbs-trans/virtio-uverbs/mlx5.h"

static int parse_hdr(const struct ib_uverbs_cmd_hdr *hdr, uint32_t *command, bool *ext,
                     bool *exp)
{
    uint32_t flags =
        (hdr->command & 0xff000000u) >> 24; // for exp (may not used)

    if (hdr->command & ~(uint32_t)(IB_USER_VERBS_CMD_FLAG_EXTENDED |
                                   IB_USER_VERBS_CMD_COMMAND_MASK))
        return -1;

    *command = hdr->command & IB_USER_VERBS_CMD_COMMAND_MASK;
    *ext = hdr->command & IB_USER_VERBS_CMD_FLAG_EXTENDED;
    *exp = !flags && (*command >= 64);
    if (*exp)
        abort();

    return 0;
}

const size_t UVERBS_QUEUE_SIZES = 256;

static char *ibverbs_on_host(const char *host)
{
    const char *fmt = "/sys/class/infiniband_verbs/%s";
    int sz = snprintf(NULL, 0, fmt, host);
    char *buf = (char *)g_malloc(sz + 1);
    snprintf(buf, sz + 1, fmt, host);
    return buf;
}

static char *ib_abi_version(void)
{
    char *abi_version_path = ibverbs_on_host("abi_version");

    FILE *f = fopen(abi_version_path, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET); /* same as rewind(f); */
    char *buf = (char *)g_malloc(fsize + 1);
    size_t read = fread(buf, 1, fsize, f);
    fclose(f);
    buf = g_realloc(buf, read + 1);
    buf[read] = '\0';
    g_free(abi_version_path);
    return buf;
}

static char *ibdev_name_of(const char *host)
{
    char *ibverbs_path = ibverbs_on_host(host);
    int sz = snprintf(NULL, 0, "%s/%s", ibverbs_path, "ibdev");
    char ibdev_path[sz + 1];
    snprintf(ibdev_path, sz + 1, "%s/%s", ibverbs_path, "ibdev");
    char *ibdev_name = (char *)g_malloc(64);
    FILE *fp = fopen(ibdev_path, "r");
    char *result = fgets(ibdev_name, 63, fp);
    assert(result != NULL);
    fclose(fp);
    g_free(ibverbs_path);

    for (int i = sz; i >= 0; --i)
    {
        if (ibdev_name[i] == '\n' || ibdev_name[i] == '\r')
        {
            ibdev_name[i] = 0;
        }
    }
    return ibdev_name;
}

static char *ibdev_on_host(const char *host)
{
    char *ibdev_name = ibdev_name_of(host);
    const char *fmt = "/sys/class/infiniband/%s";
    int sz = snprintf(NULL, 0, fmt, ibdev_name);
    char *buf = (char *)g_malloc(sz + 1);
    snprintf(buf, sz + 1, fmt, ibdev_name);
    g_free(ibdev_name);
    return buf;
}

static char *dev_on_host(char *host)
{
    const char *fmt = "/dev/infiniband/%s";
    int sz = snprintf(NULL, 0, fmt, host);
    char *buf = (char *)g_malloc(sz + 1);
    snprintf(buf, sz + 1, fmt, host);
    return buf;
}

static void populate_dir(GArray *fs_list, uint32_t ino)
{
    SysfsNode *node = &g_array_index(fs_list, SysfsNode, ino);
    if (!node->is_dir || node->entries != NULL)
        return;

    DIR *dir_ptr = NULL;
    struct dirent *file = NULL;
    struct stat buf;
    int sz;
    if ((dir_ptr = opendir(node->path)) == NULL)
    {
        abort();
    }

    // TODO::check below qemu failed when g_array is resized
    node->entries = g_array_sized_new(TRUE, TRUE, sizeof(uint32_t), 512);
    uint32_t next_ino = fs_list->len;

    while ((file = readdir(dir_ptr)) != NULL)
    {
        if (strcmp(file->d_name, ".") == 0 || strcmp(file->d_name, "..") == 0)
        {
            continue;
        }
        sz = snprintf(NULL, 0, "%s/%s", node->path, file->d_name);
        char *child_path = (char *)g_malloc(sz + 1);
        snprintf(child_path, sz + 1, "%s/%s", node->path, file->d_name);
        if (stat(child_path, &buf) == -1)
        {
            g_free(child_path);
            continue;
        }
        SysfsNode child;
        if (S_ISDIR(buf.st_mode))
        {
            child.is_dir = true;
        }
        else if (S_ISREG(buf.st_mode))
        {
            child.is_dir = false;
        }
        child.path = child_path;
        child.entries = NULL;

        g_array_append_val(fs_list, child);
        g_array_append_val(node->entries, next_ino);
        next_ino += 1;
    }

    closedir(dir_ptr);
}
static unsigned int do_process_sysfs_get_info(VirtQueueElement *elem, struct uverbs_resp *resp, uint8_t info_type, const char *host)
{
    assert(elem->in_num == 2);
    char *data = (char *)elem->in_sg[0].iov_base;
    size_t data_len = elem->in_sg[0].iov_len;
    char *buf = NULL;
    switch (info_type)
    {
    case VIRTIO_UVERBS_SYSFS_GET_INFO_ABI_VERSION:
        buf = ib_abi_version();
        break;
    case VIRTIO_UVERBS_SYSFS_GET_INFO_IBDEV_NAME:
        buf = ibdev_name_of(host);
        break;
    default:
        resp->status = VIRTIO_UVERBS_S_ERR;
        return 0;
    }
    size_t size = strlen(buf);
    assert(size < data_len);
    memcpy(data, buf, size + 1);
    g_free(buf);
    resp->status = VIRTIO_UVERBS_S_OK;
    return size + 1;
}

static unsigned int do_process_sysfs_readdir(VirtQueueElement *elem, struct uverbs_resp *resp, GArray *fs_list, uint32_t ino, uint64_t offset)
{

    if (offset < 2)
    {
        resp->status = VIRTIO_UVERBS_S_OK;
        resp->prot.readdir.error = VIRTIO_UVERBS_SYSFS_READDIR_INVAL;
        return 0;
    }

    assert(ino <= fs_list->len);
    populate_dir(fs_list, ino);

    SysfsNode node = g_array_index(fs_list, SysfsNode, ino);

    if (!node.is_dir)
    {
        resp->status = VIRTIO_UVERBS_S_OK;
        resp->prot.readdir.error = VIRTIO_UVERBS_SYSFS_READDIR_INVAL;
        return 0;
    }
    char *data = (char *)elem->in_sg[0].iov_base;
    size_t data_len = elem->in_sg[0].iov_len;

    if (offset - 2 > node.entries->len)
    {
        resp->status = VIRTIO_UVERBS_S_OK;
        resp->prot.readdir.error = VIRTIO_UVERBS_SYSFS_READDIR_NOENT;
        return 0;
    }

    uint32_t entry_ino = g_array_index(node.entries, uint32_t, offset - 2);
    SysfsNode *entry = &g_array_index(fs_list, SysfsNode, entry_ino);

    char *entry_filename = strrchr(entry->path, '/');
    ++entry_filename;

    int filename_len = strlen(entry_filename);
    int to_write = filename_len + 1 > data_len ? data_len : filename_len + 1;
    memcpy(data, entry_filename, to_write);
    data[data_len - 1] = '\0';

    resp->prot.readdir.is_dir = entry->is_dir ? 1 : 0;
    resp->status = VIRTIO_UVERBS_S_OK;
    resp->prot.readdir.error = VIRTIO_UVERBS_SYSFS_READDIR_OK;
    return to_write;
}

static unsigned int do_process_sysfs_lookup(VirtQueueElement *elem, struct uverbs_resp *resp, GArray *fs_list, uint32_t ino)
{

    assert(ino <= fs_list->len);
    populate_dir(fs_list, ino);
    assert(elem->out_num == 2);
    assert(elem->in_num == 1);
    SysfsNode node = g_array_index(fs_list, SysfsNode, ino);

    if (!node.is_dir)
    {
        abort();
    }

    char filename[elem->out_sg[1].iov_len];

    strncpy(filename, (char *)elem->out_sg[1].iov_base, elem->out_sg[1].iov_len);

    for (int i = 0; i < node.entries->len; ++i)
    {
        uint32_t entry_ino = g_array_index(node.entries, uint32_t, i);
        SysfsNode *entry = &g_array_index(fs_list, SysfsNode, entry_ino);
        char *entry_filename = strrchr(entry->path, '/');
        ++entry_filename;
        if (strcmp(filename, entry_filename) == 0)
        {
            resp->status = VIRTIO_UVERBS_S_OK;
            resp->prot.lookup.error = VIRTIO_UVERBS_SYSFS_LOOKUP_OK;
            resp->prot.lookup.ino = entry_ino;
            resp->prot.lookup.is_dir = entry->is_dir ? 1 : 0;

            return 0;
        }
    }
    resp->status = VIRTIO_UVERBS_S_OK;
    resp->prot.lookup.error = VIRTIO_UVERBS_SYSFS_LOOKUP_NOENT;
    resp->prot.lookup.ino = 0;
    resp->prot.lookup.is_dir = 0;
    return 0;
}

static unsigned int do_process_sysfs_read(VirtQueueElement *elem, struct uverbs_resp *resp, GArray *fs_list, uint32_t ino, uint64_t offset)
{
    assert(ino <= fs_list->len);
    SysfsNode node = g_array_index(fs_list, SysfsNode, ino);

    char *data = (char *)elem->in_sg[0].iov_base;
    size_t data_len = elem->in_sg[0].iov_len;

    if (node.is_dir)
    {
        resp->status = VIRTIO_UVERBS_S_OK;
        resp->prot.read.error = VIRTIO_UVERBS_SYSFS_READ_ISDIR;
        return 0;
    }

    FILE *fp = fopen(node.path, "r");
    int ret = fseek(fp, offset, SEEK_SET);
    assert(ret == 0);
    int c;
    uint32_t len = 0;
    while (data_len > 0 && (c = fgetc(fp)) != EOF)
    {
        --data_len;
        *data = c;
        ++data;
        ++len;
    }
    fclose(fp);

    resp->status = VIRTIO_UVERBS_S_OK;
    resp->prot.read.error = VIRTIO_UVERBS_SYSFS_READ_OK;

    return len;
}

static unsigned int do_process_uverbs_write(VirtQueueElement *elem, struct uverbs_resp *resp, int uverbs_fd)
{
    assert(elem->out_sg[1].iov_len >= sizeof(struct ib_uverbs_cmd_hdr));

    char buff[elem->out_sg[1].iov_len];
    memcpy(buff, elem->out_sg[1].iov_base, elem->out_sg[1].iov_len);
    struct ib_uverbs_cmd_hdr *verb_hdr = (struct ib_uverbs_cmd_hdr *)buff;

    uint32_t command = 0;
    bool ext = false;
    bool exp = false;
    int ret = parse_hdr(verb_hdr, &command, &ext, &exp);

    assert(ret == 0);

    if (exp)
        ibverbs_translate_exp(command, buff);
    else if (ext)
        ibverbs_translate_ext(command, buff);
    else
        ibverbs_translate_general(command, buff);

    // Provider specific
    if (exp)
        mlx5_translate_exp(command, buff);
    else if (ext)
        mlx5_translate_ext(command, buff);
    else
        mlx5_translate_general(command, buff);

    if (verb_hdr->out_words != 0)
    {
        uint64_t *response = (uint64_t *)(buff + sizeof(struct ib_uverbs_cmd_hdr));
        *response = (uint64_t)elem->in_sg[0].iov_base;
    }

    size_t remain = elem->out_sg[1].iov_len;
    ssize_t done = 0;
    char *buff_ptr = (char *)buff;
    while (remain > 0)
    {
        done = write(uverbs_fd, buff_ptr, remain);
        if (done < 0)
        {
            printf("WRITE ERRPR: %d %s\n", errno, strerror(errno));
            printf("WRITE uverbs_fd: %d, buff_ptr: %p, remain: %lx\n", uverbs_fd, buff_ptr, remain);
        }
        assert(done == remain);
        assert(done >= 0);
        remain -= done;
        buff_ptr += done;
    }
    resp->status = VIRTIO_UVERBS_S_OK;
    return elem->out_sg[1].iov_len;
}

static unsigned int do_process_uverbs_mmap(VirtQueueElement *elem, struct uverbs_resp *resp, int uverbs_fd, uint32_t offset, int32_t prot, int32_t flags)
{

    int ret1 = munmap(elem->in_sg[0].iov_base, elem->in_sg[0].iov_len);
    assert(ret1 == 0);
    void *ret = mmap(elem->in_sg[0].iov_base, elem->in_sg[0].iov_len, prot, flags | MAP_FIXED, uverbs_fd, offset);

    if (ret == MAP_FAILED)
    {
        resp->status = VIRTIO_UVERBS_S_OK;
        resp->prot.mmap.error = VIRTIO_UVERBS_DEV_MMAP_FAIL;
        return 0;
    }

    resp->status = VIRTIO_UVERBS_S_OK;
    resp->prot.mmap.error = VIRTIO_UVERBS_DEV_MMAP_OK;
    return 0;
}
static unsigned int do_process_uverbs_munmap(VirtQueueElement *elem, struct uverbs_resp *resp, int uverbs_fd)
{
    int ret = munmap(elem->out_sg[1].iov_base, elem->out_sg[1].iov_len);

    if (ret == -1)
    {
        resp->status = VIRTIO_UVERBS_S_OK;
        resp->prot.mmap.error = VIRTIO_UVERBS_DEV_MUNMAP_FAIL;
        return 0;
    }

    resp->status = VIRTIO_UVERBS_S_OK;
    resp->prot.mmap.error = VIRTIO_UVERBS_DEV_MUNMAP_OK;

    return 0;
}

static void do_process_uverbs(VirtIOUverbs *vuverbs)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(vuverbs);

    VirtQueueElement *elem;

    while (!virtio_queue_empty(vuverbs->uverbsq))
    {

        struct uverbs_hdr hdr;
        struct uverbs_resp resp;
        unsigned int len = 0;
        // START PROCESSING HEADER
        elem = virtqueue_pop(vuverbs->uverbsq, sizeof(VirtQueueElement));
        if (elem == NULL)
            break;
        assert(elem->out_sg[0].iov_len == sizeof(hdr));
        memcpy(&hdr, elem->out_sg[0].iov_base, elem->out_sg[0].iov_len);
        // END PROCESSING HEADER

        memset(&resp, 0, sizeof(resp));

        resp.type = hdr.type;

        GArray *fs_list = NULL;
        switch (hdr.type)
        {
        case VIRTIO_UVERBS_SYSFS_GET_INFO:
            len = do_process_sysfs_get_info(elem, &resp, hdr.prot.get_info.info_type, vuverbs->conf.host);
            break;
        case VIRTIO_UVERBS_SYSFS_READDIR:
            assert(hdr.prot.readdir.fsno <= vuverbs->sysfs_path_list->len);
            fs_list = g_array_index(vuverbs->sysfs_path_list, GArray *, hdr.prot.readdir.fsno);
            len = do_process_sysfs_readdir(elem, &resp, fs_list, hdr.prot.readdir.ino, hdr.prot.readdir.offset);
            break;
        case VIRTIO_UVERBS_SYSFS_LOOKUP:
            assert(hdr.prot.lookup.fsno <= vuverbs->sysfs_path_list->len);
            fs_list = g_array_index(vuverbs->sysfs_path_list, GArray *, hdr.prot.lookup.fsno);
            len = do_process_sysfs_lookup(elem, &resp, fs_list, hdr.prot.lookup.ino);
            break;
        case VIRTIO_UVERBS_SYSFS_READ:
            assert(hdr.prot.read.fsno <= vuverbs->sysfs_path_list->len);
            fs_list = g_array_index(vuverbs->sysfs_path_list, GArray *, hdr.prot.read.fsno);
            len = do_process_sysfs_read(elem, &resp, fs_list, hdr.prot.read.ino, hdr.prot.read.offset);
            break;

        case VIRTIO_UVERBS_DEV_WRITE:
            len = do_process_uverbs_write(elem, &resp, vuverbs->uverbs_fd);
            break;
        case VIRTIO_UVERBS_DEV_MMAP:
            len = do_process_uverbs_mmap(elem, &resp, vuverbs->uverbs_fd, hdr.prot.mmap.offset, hdr.prot.mmap.prot, hdr.prot.mmap.flags);
            break;
        case VIRTIO_UVERBS_DEV_MUNMAP:
            len = do_process_uverbs_munmap(elem, &resp, vuverbs->uverbs_fd);
            break;
        default:
            resp.status = VIRTIO_UVERBS_S_UNSUPP;
            abort();
        }

        assert(elem->in_num > 0);
        assert(elem->in_sg[elem->in_num - 1].iov_len == sizeof(resp));
        memcpy(elem->in_sg[elem->in_num - 1].iov_base, &resp, sizeof(resp));

        virtqueue_push(vuverbs->uverbsq, elem, len);
        g_free(elem);
    }
    virtio_notify(vdev, vuverbs->uverbsq);
}

static bool is_uverbs_ready(VirtIOUverbs *vuverbs)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(vuverbs);
    if (virtio_queue_ready(vuverbs->uverbsq) && (vdev->status & VIRTIO_CONFIG_S_DRIVER_OK))
    {
        return true;
    }
    return false;
}
static void virtio_uverbs_process(VirtIOUverbs *vuverbs)
{
    if (is_uverbs_ready(vuverbs))
    {
        do_process_uverbs(vuverbs);
    }
}

static void handle_input_uverbs(VirtIODevice *vdev, VirtQueue *vq)
{
    VirtIOUverbs *vuverbs = VIRTIO_UVERBS(vdev);
    do_process_uverbs(vuverbs);
}

static uint64_t virtio_uverbs_get_features(VirtIODevice *vdev, uint64_t f, Error **errp)
{
    return f;
}

static void virtio_uverbs_vm_state_change(void *opaque, int running,
                                          RunState state)
{
    VirtIOUverbs *vuverbs = opaque;

    //trace_virtio_uverbs_vm_state_change(vuverbs, running, state);

    /* We may have an element ready but couldn't process it due to a quota
     * limit or because CPU was stopped.  Make sure to try again when the
     * CPU restart.
     */

    if (running)
    {
        virtio_uverbs_process(vuverbs);
    }
}

static void virtio_uverbs_set_status(VirtIODevice *vdev, uint8_t status)
{
    VirtIOUverbs *vuverbs = VIRTIO_UVERBS(vdev);

    if (!vdev->vm_running)
    {
        return;
    }
    vdev->status = status;

    /* Something changed, try to process buffers */
    virtio_uverbs_process(vuverbs);
}

static void virtio_uverbs_device_realize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOUverbs *vuverbs = VIRTIO_UVERBS(dev);

    if (vuverbs->conf.host == NULL)
    {
        error_setg(errp, "'host' parameter expects a valid object");
        return;
    }

    virtio_init(vdev, "virtio-uverbs", VIRTIO_ID_UVERBS, 0);

    vuverbs->uverbsq = virtio_add_queue(vdev, UVERBS_QUEUE_SIZES, handle_input_uverbs);

    vuverbs->vmstate = qemu_add_vm_change_state_handler(virtio_uverbs_vm_state_change,
                                                        vuverbs);

    vuverbs->sysfs_path_list = g_array_sized_new(TRUE, FALSE, sizeof(GArray *), 2);

    GArray *ibverbs_list = g_array_sized_new(TRUE, FALSE, sizeof(SysfsNode), 512);
    SysfsNode ibverbs_root;
    ibverbs_root.is_dir = true;
    ibverbs_root.path = ibverbs_on_host(vuverbs->conf.host);
    ibverbs_root.entries = NULL;
    g_array_append_val(ibverbs_list, ibverbs_root);

    GArray *ibdev_list = g_array_sized_new(TRUE, FALSE, sizeof(SysfsNode), 512);
    SysfsNode ibdev_root;
    ibdev_root.is_dir = true;
    ibdev_root.path = ibdev_on_host(vuverbs->conf.host);
    ibdev_root.entries = NULL;
    g_array_append_val(ibdev_list, ibdev_root);

    g_array_append_val(vuverbs->sysfs_path_list, ibverbs_list);
    g_array_append_val(vuverbs->sysfs_path_list, ibdev_list);

    // open device

    char *devpath = dev_on_host(vuverbs->conf.host);
    vuverbs->uverbs_fd = open(devpath, O_RDWR);
    assert(vuverbs->uverbs_fd > 0);
    g_free(devpath);
}

static void virtio_uverbs_device_unrealize(DeviceState *dev, Error **errp)
{
    VirtIODevice *vdev = VIRTIO_DEVICE(dev);
    VirtIOUverbs *vuverbs = VIRTIO_UVERBS(dev);

    qemu_del_vm_change_state_handler(vuverbs->vmstate);
    virtio_cleanup(vdev);
}

static const VMStateDescription vmstate_virtio_uverbs = {
    .name = "virtio-uverbs",
    .minimum_version_id = 1,
    .version_id = 1,
    .fields = (VMStateField[]){
        VMSTATE_VIRTIO_DEVICE,
        VMSTATE_END_OF_LIST()},
};

static Property virtio_uverbs_properties[] = {
    DEFINE_PROP_STRING("host", VirtIOUverbs, conf.host),
    DEFINE_PROP_END_OF_LIST(),
};

static void virtio_uverbs_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioDeviceClass *vdc = VIRTIO_DEVICE_CLASS(klass);

    dc->props = virtio_uverbs_properties;
    dc->vmsd = &vmstate_virtio_uverbs;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
    vdc->realize = virtio_uverbs_device_realize;
    vdc->unrealize = virtio_uverbs_device_unrealize;
    vdc->get_features = virtio_uverbs_get_features;
    vdc->set_status = virtio_uverbs_set_status;
}

static const TypeInfo virtio_uverbs_info = {
    .name = TYPE_VIRTIO_UVERBS,
    .parent = TYPE_VIRTIO_DEVICE,
    .instance_size = sizeof(VirtIOUverbs),
    .class_init = virtio_uverbs_class_init,
};

static void virtio_register_types(void)
{
    type_register_static(&virtio_uverbs_info);
}

type_init(virtio_register_types)
