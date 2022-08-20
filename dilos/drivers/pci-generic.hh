/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef PCI_GENERIC_HH
#define PCI_GENERIC_HH

#include <stdint.h>
#include <osv/types.h>

#define pci_tag "pci"
#define pci_d(...)   tprintf_d(pci_tag, __VA_ARGS__)
#define pci_i(...)   tprintf_i(pci_tag, __VA_ARGS__)
#define pci_w(...)   tprintf_w(pci_tag, __VA_ARGS__)
#define pci_e(...)   tprintf_e(pci_tag, __VA_ARGS__)

namespace pci {
    enum pc_early_defines {
        PCI_VENDOR_ID      = 0x0,
        PCI_CONFIG_ADDRESS = 0xcf8,
        PCI_CONFIG_DATA    = 0xcfc,
        PCI_BUS_OFFSET     = 16,
        PCI_SLOT_OFFSET    = 11,
        PCI_FUNC_OFFSET    = 8,
        PCI_CONFIG_ADDRESS_ENABLE = 0x80000000,
        PCI_COMMAND_OFFSET = 0x4,
        PCI_BUS_MASTER_BIT = 0x2,
        PCI_STATUS_OFFSET  = 0x6,
        PCI_CLASS_REVISION = 0x8,
        PCI_CLASS_OFFSET   = 0xb,
        PCI_SUBCLASS_OFFSET= 0xa,
        PCI_HEADER_TYPE    = 0xe,
        PCI_SUBSYSTEM_ID   = 0x2e,
        PCI_SUBSYSTEM_VID  = 0x2c,
        PCI_HEADER_MULTI_FUNC = 0x80,
        PCI_BAR0_ADDR      = 0x10,
        PCI_CONFIG_SECONDARY_BUS = 0x19,
        PCI_CAPABILITIES_PTR = 0x34,
    };

    /* these ones are implemented in the generic pci code in drivers/pci.cc */
    void pci_device_print(u8 bus, u8 slot, u8 func);
    void pci_devices_print();
    void pci_device_enumeration();

} /* namespace pci */

#endif /* PCI_GENERIC_HH */
