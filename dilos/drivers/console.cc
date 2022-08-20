/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */


#include <osv/prex.h>
#include <osv/device.h>
#include <osv/debug.hh>
#include <osv/prio.hh>
#include <queue>
#include <deque>
#include <vector>
#include <sys/ioctl.h>
#include "console.hh"
#include "console-multiplexer.hh"
#include "early-console.hh"

#include <termios.h>
#include <signal.h>

#include <fs/fs.hh>

namespace console {

termios tio = {
    .c_iflag = ICRNL,
    .c_oflag = OPOST | ONLCR,
    .c_cflag = 0,
    .c_lflag = ECHO | ECHOCTL | ICANON | ECHOE | ISIG,
    .c_line = 0,
    .c_cc = {/*VINTR*/'\3', /*VQUIT*/'\34', /*VERASE*/'\177', /*VKILL*/0,
            /*VEOF*/0, /*VTIME*/0, /*VMIN*/0, /*VSWTC*/0,
            /*VSTART*/0, /*VSTOP*/0, /*VSUSP*/0, /*VEOL*/0,
            /*VREPRINT*/0, /*VDISCARD*/0, /*VWERASE*/0,
            /*VLNEXT*/0, /*VEOL2*/0},
};

struct winsize ws = {
    .ws_row = 25,
    .ws_col = 80,
};

console_multiplexer mux __attribute__((init_priority((int)init_prio::console)))
    (&tio, &arch_early_console);

void write(const char *msg, size_t len)
{
    if (len == 0)
        return;

    mux.write(msg, len);
}

// lockless version
void write_ll(const char *msg, size_t len)
{
    if (len == 0)
        return;

    mux.write_ll(msg, len);
}

static int
console_ioctl(u_long request, void *arg)
{
    switch (request) {
    case TCGETS:
        *static_cast<termios*>(arg) = tio;
        return 0;
    case TCSETS:
        // FIXME: We may need to lock this, since the writers are
        // still consulting the data. But for now, it is not terribly
        // important
        tio = *static_cast<termios*>(arg);
        if (!(tio.c_lflag & ICANON)) {
            // Going into raw (non-canonical) mode, so make whatever is in the
            // edit buffer available for immediate reading. It will continue
            // to be available even even after we return to canonical mode.
            mux.take_pending_input();
        }
        return 0;
    case TCSETSF:
        tio = *static_cast<termios*>(arg);
        mux.discard_pending_input();
        return 0;
    case TCFLSH:
        switch ((int)(intptr_t)arg) {
        case TCIFLUSH:
            mux.discard_pending_input();
            return 0;
        default:
            return -EINVAL;
        }
    case TIOCGWINSZ:
        *static_cast<struct winsize*>(arg) = ws;
        return 0;
    case FIONREAD:
        // used in OpenJDK's os::available (os_linux.cpp)
        *static_cast<int*>(arg) = mux.read_queue_size();
        return 0;
    default:
        return -ENOTTY;
    }
}

template <class T> static int
op_read(T *ignore, struct uio *uio, int ioflag)
{
    mux.read(uio, ioflag);
    return 0;
}

template <class T> static int
op_write(T *ignore, struct uio *uio, int ioflag)
{
    mux.write(uio, ioflag);
    return 0;
}

template <class T> static int
op_ioctl(T *ignore, u_long request, void *arg)
{
    return console_ioctl(request, arg);
}

static struct devops console_devops = {
    .open	= no_open,
    .close	= no_close,
    .read	= op_read<device>,
    .write	= op_write<device>,
    .ioctl	= op_ioctl<device>,
    .devctl	= no_devctl,
};

struct driver console_drv = {
    .name	= "console",
    .devops	= &console_devops,
};

void console_driver_add(console_driver *driver)
{
    mux.driver_add(driver);
}

void console_init()
{
    device_create(&console_drv, "console", D_CHR | D_TTY);
    mux.start();
}

class console_file : public tty_file {
public:
    console_file() : tty_file(FREAD|FWRITE, DTYPE_UNSPEC) {}
    virtual int read(struct uio *uio, int flags) override;
    virtual int write(struct uio *uio, int flags) override;
    virtual int ioctl(u_long com, void *data) override;
    virtual int stat(struct stat* buf) override;
    virtual int close() override;
    virtual int poll(int events) override;
};

// The above allows opening /dev/console to use the console. We currently
// have a bug with the VFS/device layer - vfs_read() and vfs_write() hold a
// lock throughout the call, preventing a write() to the console while read()
// is blocked. Moreover, poll() can't be implemented on files, including
// character special devices.
// To bypass this buggy layer, here's an console::open() function, a
// replacement for open("/dev/console", O_RDWR, 0).

int console_file::stat(struct stat *s)
{
    // We are not expected to fill much in s. Java expects (see os::available
    // in os_linux.cpp) S_ISCHR to be true.
    // TODO: what does Linux fill when we stat() a tty?
    memset(s, 0, sizeof(struct stat));
    s->st_mode = S_IFCHR;
    return 0;
}

int console_file::read(struct uio *uio, int flags)
{
    return op_read<file>(this, uio, flags);
}

int console_file::write(struct uio *uio, int flags)
{
    return op_write<file>(this, uio, flags);
}

int console_file::ioctl(u_long com, void *data)
{
    return op_ioctl<file>(this, com, data);
}

int console_file::close()
{
    return 0;
}

int console_file::poll(int events) {
    // We do not support flow-control (so the console is always writable)
    // and do not support hangup on the console.
    int ret = POLLOUT;
    if (mux.read_queue_size() > 0) {
        ret |= POLLIN;
    }
    return ret & events;
}

int open() {
    try {
        // Keep a single file structure for the console, even if opened
        // multiple times (we can still have multiple fds, of course).
        static fileref console_fr;
        static mutex open_mutex;
        WITH_LOCK(open_mutex) {
            if (!console_fr) {
                console_fr = make_file<console_file>();
                mux.set_fp(console_fr.get());
            }
            fdesc fd(console_fr);
            return fd.release();
        }
    } catch (int error) {
        errno = error;
        return -1;
    }
}

}
