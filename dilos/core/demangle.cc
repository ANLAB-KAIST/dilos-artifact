/*
 * Copyright (C) 2014 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <string.h>
#include <algorithm>
#include <osv/elf.hh>
#include <cxxabi.h>

#include <osv/demangle.hh>

namespace osv {

extern "C" int __gcclibcxx_demangle_callback (const char *,
    void (*)(const char *, size_t, void *),
    void *);

struct demangle_callback_arg {
    char *buf;
    size_t len;
};

static void demangle_callback(const char *buf, size_t len, void *opaque)
{
    struct demangle_callback_arg *arg =
        reinterpret_cast<struct demangle_callback_arg *>(opaque);
    len = std::min(len, arg->len - 1);
    strncpy(arg->buf, buf, len);
    arg->buf[len] = '\0';
    arg->buf += len;
    arg->len -= len;
}

bool demangle(const char *name, char *buf, size_t len)
{
    struct demangle_callback_arg arg { buf, len };
    int ret = __gcclibcxx_demangle_callback(name, demangle_callback,
            &arg);
    return (ret == 0);
}

void lookup_name_demangled(void *addr, char *buf, size_t len)
{
    auto ei = elf::get_program()->lookup_addr(addr);
    int funclen;

    if (!ei.sym)
        strncpy(buf, "???", len);
    else if (!demangle(ei.sym, buf, len))
        strncpy(buf, ei.sym, len);
    funclen = strlen(buf);
    snprintf(buf + funclen, len - funclen, "+%d",
        reinterpret_cast<uintptr_t>(addr)
        - reinterpret_cast<uintptr_t>(ei.addr));
}

std::unique_ptr<char> demangle(const char *name)
{
    int status;
    char *demangled = abi::__cxa_demangle(name, nullptr, 0, &status);
    // Note: __cxa_demangle used malloc() to allocate demangled, and we'll
    // need to use free() to free it. Here we're assuming that unique_ptr's
    // default deallocator, "delete", is the same thing as free().
    return std::unique_ptr<char>(demangled);
}

demangler::~demangler()
{
    free(_buf);
}
const char *demangler::operator()(const char * name)
{
    int status;
    auto *demangled = abi::__cxa_demangle(name, _buf, &_len, &status);
    if (demangled) {
        _buf = demangled;
        return _buf;
    } else {
        return nullptr;
    }
}

}
