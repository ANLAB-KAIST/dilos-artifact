/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "drivers/driver.hh"
#include <osv/pci.hh>
#include <osv/debug.hh>

#include "driver.hh"

using namespace pci;

namespace hw {

    driver_manager* driver_manager::_instance = nullptr;

    driver_manager::driver_manager()
    {

    }

    driver_manager::~driver_manager()
    {
        unload_all();
    }

    void driver_manager::register_driver(std::function<hw_driver* (hw_device*)> probe)
    {
        _probes.push_back(probe);
    }

    void driver_manager::load_all()
    {
        auto dm = device_manager::instance();
        dm->for_each_device([this] (hw_device* dev) {
            for (auto probe : _probes) {
                if (auto drv = probe(dev)) {
                    dev->set_attached();
                    _drivers.push_back(drv);
                    break;
                }
            }
        });
    }

    void driver_manager::unload_all()
    {
        for (auto drv : _drivers) {
            delete drv;
        }
        _drivers.clear();
    }

    void driver_manager::list_drivers()
    {
        for (auto drv : _drivers) {
            drv->dump_config();
        }
    }
}
