/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef DRIVER_H
#define DRIVER_H

#include <osv/pci.hh>
#include "drivers/pci-device.hh"
#include <vector>
#include <string>

namespace hw {

    class hw_driver {
    public:
        virtual ~hw_driver() {};

        // Drivers are indexed by their names
        virtual std::string get_name() const = 0;

        // virtual bool sleep() = 0;
        // virtual bool wake() = 0;
        // ...

        virtual void dump_config() = 0;
    };

    class driver_manager {
    public:
        driver_manager();
        virtual ~driver_manager();

        static driver_manager* instance() {
            if (_instance == nullptr) {
                _instance = new driver_manager();
            }

            return _instance;
        }

        void register_driver(std::function<hw_driver* (hw_device*)> probe);
        void load_all();
        void unload_all();
        void list_drivers();

    private:
        static driver_manager* _instance;
        std::vector<std::function<hw_driver* (hw_device*)>> _probes;
        std::vector<hw_driver*> _drivers;
    };
}


#endif
