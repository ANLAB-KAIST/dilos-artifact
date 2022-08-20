/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef _OSV_BSD_KTHREAD_H
#define _OSV_BSD_KTHREAD_H

#include <sys/cdefs.h>

#ifdef _KERNEL

// Compat only
#define RFHIGHPID   (1<<18) /* use a pid higher than 10 (idleproc) */

__BEGIN_DECLS

struct proc {;
    void *_bogus;
};

int
kproc_create(void (*func)(void *), void *arg,
                    struct proc **newpp, int flags, int pages, const char *fmt, ...);

struct proc *get_curproc(void);
extern void thread_mark_emergency();
__END_DECLS

#define curproc (get_curproc())

extern unsigned char *osv_reclaimer_thread;
#define pageproc ((struct proc *)osv_reclaimer_thread)

#endif

#endif
