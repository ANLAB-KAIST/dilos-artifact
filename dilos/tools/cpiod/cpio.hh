/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */


#ifndef OSV_TOOLS_CPIOD_CPIO_HH_
#define OSV_TOOLS_CPIOD_CPIO_HH_

#include <istream>
#include <string>
#include <sys/stat.h>

namespace osv {

class cpio_in {
public:
    virtual ~cpio_in();
    virtual void add_file(std::string name, std::istream& is, mode_t mode) = 0;
    virtual void add_dir(std::string name, mode_t mode) = 0;
    virtual void add_symlink(std::string oldpath, std::string newpath, mode_t mode) = 0;
public:
    static void parse(std::istream& is, cpio_in& out);
private:
    static bool parse_one(std::istream& is, cpio_in& out);
};

}

#endif /* OSV_TOOLS_CPIOD_CPIO_HH_ */
