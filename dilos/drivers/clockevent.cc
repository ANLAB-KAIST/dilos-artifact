/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "drivers/clockevent.hh"

clock_event_driver* clock_event;

clock_event_driver::~clock_event_driver()
{
}

void clock_event_driver::set_callback(clock_event_callback* cb)
{
    _callback = cb;
}

clock_event_callback* clock_event_driver::callback() const
{
    return _callback;
}

clock_event_callback::~clock_event_callback()
{
}
