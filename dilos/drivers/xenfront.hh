/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef XENFRONT_DRIVER_H
#define XENFRONT_DRIVER_H

#include "driver.hh"
#include <osv/pci.hh>
#include "drivers/driver.hh"
#include "drivers/pci-function.hh"
#include "drivers/pci-device.hh"
#include <osv/interrupt.hh>
#include <osv/device.h>
#include <bsd/porting/bus.h>
#include <xen/interface/io/xenbus.h>

struct xenbus_device_ivars;

template <typename T>
inline T *bsd_to_dev(struct device *bd)
{
    T *base = NULL;
    return reinterpret_cast<T *>((unsigned long)bd - (unsigned long)&base->_bsd_dev);
}

namespace xenfront {

typedef void (*probe)(device_t dev);
typedef int  (*attach)(device_t dev);
typedef int  (*detach)(device_t dev);
typedef void (*backend_changed)(device_t dev, XenbusState backend_state);

class xenbus;

class xenfront_driver {

public:
    explicit xenfront_driver(xenbus *bus);

    std::string get_name() const { return _driver_name; }

    const std::string &get_type() { return _type; }
    const std::string &get_node_path() { return _node_path; }
    const std::string &get_otherend_path() { return _otherend_path; };

    const int get_otherend_id() { return _otherend_id; }
    void set_ivars(struct xenbus_device_ivars *ivars);

    void localend_changed(std::string local) { };
    void otherend_changed(XenbusState state);
    int attach();
    int detach();
    void probe();
    void finished();

    bool operator==(std::string str) { return _node_path == str; }
    static xenfront_driver *from_device(struct device *dev) { return bsd_to_dev<xenfront_driver>(dev); }
    struct device _bsd_dev;
protected:
    xenbus *_bus;
    std::string _driver_name;
    std::string _node_path;
    std::string _otherend_path;
    std::string _type;
    int _otherend_id;
    unsigned int _irq;
    int _irq_type;

    xenfront::probe _probe = nullptr;
    xenfront::attach _attach = nullptr;
    xenfront::detach _detach = nullptr;
    xenfront::backend_changed _backend_changed = nullptr;
};
}

#endif

