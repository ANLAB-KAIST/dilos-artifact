/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include "plugin.hh"
#include "autogen/plugin.json.hh"

namespace httpserver {

namespace api {

namespace plugin {

using namespace std;
using namespace json;
using namespace plugin_json;

extern "C" void init(void* arg)
{

    plugin_json_init_path("plugin extension API");

    plugin_hello.set_handler([](const_req req){
        return "hello";
    });

}

}
}
}
