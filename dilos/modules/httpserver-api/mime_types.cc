//
// mime_types.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "mime_types.hh"

namespace http {

namespace server {

namespace mime_types {

struct mapping {
    const char* extension;
    const char* mime_type;
} mappings[] = {
    { "gif",  "image/gif" },
    { "htm",  "text/html" },
    { "css",  "text/css" },
    { "js",   "text/javascript" },
    { "html", "text/html" },
    { "jpg",  "image/jpeg" },
    { "png",  "image/png" },
    { "txt",  "text/plain" },
    { "ico",  "image/x-icon" },
    { "bin",  "application/octet-stream" },
    { "json",  "application/json" },
};

std::string extension_to_type(const std::string& extension)
{
    for (mapping m : mappings) {
        if (m.extension == extension) {
            return m.mime_type;
        }
    }
    return "text/plain";
}

} // namespace mime_types

} // namespace server

} // namespace http
