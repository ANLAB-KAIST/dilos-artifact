#include "qemu/osdep.h"

#include "virtio-pci.h"
#include "hw/virtio/virtio-uverbs.h"
#include "qapi/error.h"
#include "qemu/module.h"


typedef struct VirtIOUverbsPCI VirtIOUverbsPCI;

/*
 * virtio-uverbs-pci: This extends VirtioPCIProxy.
 */
#define TYPE_VIRTIO_UVERBS_PCI "virtio-uverbs-pci-base"
#define VIRTIO_UVERBS_PCI(obj) \
        OBJECT_CHECK(VirtIOUverbsPCI, (obj), TYPE_VIRTIO_UVERBS_PCI)

struct VirtIOUverbsPCI {
    VirtIOPCIProxy parent_obj;
    VirtIOUverbs vdev;
};

static void virtio_uverbs_pci_realize(VirtIOPCIProxy *vpci_dev, Error **errp)
{
    VirtIOUverbsPCI *vuverbs = VIRTIO_UVERBS_PCI(vpci_dev);
    DeviceState *vdev = DEVICE(&vuverbs->vdev);
    Error *err = NULL;

    qdev_set_parent_bus(vdev, BUS(&vpci_dev->bus));
    object_property_set_bool(OBJECT(vdev), true, "realized", &err);
    if (err) {
        error_propagate(errp, err);
        return;
    }
}

static void virtio_uverbs_pci_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    VirtioPCIClass *k = VIRTIO_PCI_CLASS(klass);
    PCIDeviceClass *pcidev_k = PCI_DEVICE_CLASS(klass);

    k->realize = virtio_uverbs_pci_realize;
    set_bit(DEVICE_CATEGORY_MISC, dc->categories);

    pcidev_k->vendor_id = PCI_VENDOR_ID_REDHAT_QUMRANET;
    pcidev_k->device_id = PCI_DEVICE_ID_VIRTIO_UVERBS;
    pcidev_k->revision = VIRTIO_PCI_ABI_VERSION;
    pcidev_k->class_id = PCI_CLASS_OTHERS;
}

static void virtio_uverbs_initfn(Object *obj)
{
    VirtIOUverbsPCI *dev = VIRTIO_UVERBS_PCI(obj);

    virtio_instance_init_common(obj, &dev->vdev, sizeof(dev->vdev),
                                TYPE_VIRTIO_UVERBS);
}

static const VirtioPCIDeviceTypeInfo virtio_uverbs_pci_info = {
    .base_name             = TYPE_VIRTIO_UVERBS_PCI,
    .generic_name          = "virtio-uverbs-pci",
    .transitional_name     = "virtio-uverbs-pci-transitional",
    .non_transitional_name = "virtio-uverbs-pci-non-transitional",
    .instance_size = sizeof(VirtIOUverbsPCI),
    .instance_init = virtio_uverbs_initfn,
    .class_init    = virtio_uverbs_pci_class_init,
};

static void virtio_uverbs_pci_register(void)
{
    virtio_pci_types_register(&virtio_uverbs_pci_info);
}

type_init(virtio_uverbs_pci_register)
