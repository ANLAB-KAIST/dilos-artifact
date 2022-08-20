/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef FS_HH
#define FS_HH

#include <dirent.h>
#include <string>
#include <cstdint>
#include <boost/intrusive_ptr.hpp>
#include <osv/file.h>
#include <utility>

static inline void intrusive_ptr_add_ref(file *fp)
{
    fhold(fp);
}

static inline void intrusive_ptr_release(file *fp)
{
    fdrop(fp);
}

typedef boost::intrusive_ptr<file> fileref;

namespace std {

template<>
struct hash<fileref> {
    size_t operator()(const fileref& fp) const {
        return hash<file*>()(fp.get());
    }
};

}

fileref fileref_from_fd(int fd);
fileref fileref_from_fname(std::string name);
uint64_t size(fileref f);
void read(fileref f, void *buffer, uint64_t offset, uint64_t len);
void write(fileref f, const void* buffer, uint64_t offset, uint64_t len);

template <class file_type, typename... args>
fileref
make_file(args&&... a)
{
    file* fp = new (std::nothrow) file_type(std::forward<args>(a)...);
    if (!fp) {
        throw ENOMEM;
    }
    return fileref(fp, false);
}

class fdesc {
public:
    explicit fdesc() : _fd(-1) {}
    explicit fdesc(int fd) : _fd(fd) {}
    explicit fdesc(file* f);
    explicit fdesc(const fileref& f) : fdesc(f.get()) {}
    // fdesc(fileref&& f) would be nice, but we can't implement it
    ~fdesc();
    int get() { return _fd; }
    int release() { int fd = -1; std::swap(fd, _fd); return fd; }
private:
    int _fd;
};

#endif
