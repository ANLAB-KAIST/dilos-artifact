#pragma once

#include <string>

namespace ddc {

namespace elf {
void register_app(const std::string &app);
void register_skip_app(const std::string &app);
void register_replacement(void *from, void *to, void **next);
bool rewrite_symbol(const std::string &app, void **addr);
}  // namespace elf

extern void (*ddc_free)(void *);
}  // namespace ddc

#define REPLACEMENT_PREFIX(prefix, symbol) \
    ::ddc::elf::register_replacement(      \
        reinterpret_cast<void *>(symbol),  \
        reinterpret_cast<void *>(prefix##_##symbol), NULL)
