#include <osv/mutex.h>

#include <map>
#include <osv/debug.hh>
#include <set>

#include "init.hh"

namespace ddc {
namespace elf {

struct replacement_t {
    void *to;
    void **next;
};

static mutex_t lock;
static std::set<std::string> apps;
static std::map<uintptr_t, replacement_t> replacements;
static std::set<std::string> skip_apps = {"/libenviron.so",
                                          "/libvdso.so",
                                          "/tools/mount-fs.so",
                                          "/tools/umount.so",
                                          "/usr/lib/libgcc_s.so.1",
                                          "/libuutil.so",
                                          "/zpool.so",
                                          "/libzfs.so",
                                          "/zfs.so",
                                          "/tools/mkfs.so",
                                          "/tools/cpiod.so",
                                          "/usr/lib/libsqlite3.so.0"};

void register_app(const std::string &app) {
    SCOPE_LOCK(lock);
    apps.insert(app);
    apps.insert("/" + app);
}

void register_skip_app(const std::string &app) {
    SCOPE_LOCK(lock);
    skip_apps.insert(app);
    skip_apps.insert("/" + app);
}

bool rewrite_symbol(const std::string &app, void **addr) {
    SCOPE_LOCK(lock);

    if (skip_apps.find(app) != skip_apps.end()) {
        return false;
    }

    auto replacement = replacements.find(*reinterpret_cast<uintptr_t *>(addr));
    if (replacement != replacements.end()) {
        if (replacement->second.next) {
            *(replacement->second.next) = *addr;
        }
        *addr = reinterpret_cast<void *>(replacement->second.to);
        return true;
    }

    return false;
}

void register_replacement(void *from, void *to, void **next) {
    SCOPE_LOCK(lock);
    replacements[reinterpret_cast<uintptr_t>(from)] = {to, next};
}

}  // namespace elf

void elf_replacer_init() { malloc_init(); }
}  // namespace ddc
