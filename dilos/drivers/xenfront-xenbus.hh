/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef XENFRONT_BUS_DRIVER_H
#define XENFRONT_BUS_DRIVER_H

#include "drivers/xenfront.hh"
#include "drivers/device.hh"
#include <osv/device.h>
#include <bsd/porting/bus.h>

namespace xenfront {
    class xenbus : public hw_driver {
    public:

        explicit xenbus();
        static hw_driver* probe(hw_device* dev);

        virtual void dump_config();
        virtual std::string get_name() const { return _driver_name; }
        const std::string &get_node_path() { return _node_path; }

        int num_children() { return _children.size(); }
        void for_each_child(std::function<void(xenfront_driver *dev)> func);

        void remove_pending(xenfront_driver *dev);
        void add_pending(xenfront_driver *dev);

        static xenbus *instance() { return _instance; }
        static xenbus *from_device(struct device *dev) { return bsd_to_dev<xenbus>(dev); }
        struct device _bsd_dev;
    private:
        static struct xenbus *_instance;
        void wait_for_devices();
        struct device _xenstore_device;

        std::vector<xenfront_driver *> _children;
        mutex_t _children_mutex;

        sched::thread *_main_thread;
        std::string _driver_name;
        std::string _node_path;
        int _evtchn;
        int _domid;

        condvar_t _pending_devices;
        std::list<xenfront_driver *> _pending_children;
    };
}

#endif

