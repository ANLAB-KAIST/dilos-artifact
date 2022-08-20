/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef __TST_DEVICES__
#define __TST_DEVICES__

#include "tst-hub.hh"
#include <osv/pci.hh>
#include "drivers/driver.hh"
#include <osv/debug.hh>

class test_devices : public unit_tests::vtest {

public:

    void run()
    {
            // Comment it out to reduce output
            pci::pci_devices_print();

            // List all devices
            hw::device_manager::instance()->list_devices();

            debug("Devices test succeeded\n");
    }
};

#endif
