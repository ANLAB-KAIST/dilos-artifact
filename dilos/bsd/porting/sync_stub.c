/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#include <osv/mutex.h>
#include <osv/rwlock.h>
#include <stdlib.h>

#include <bsd/porting/sync_stub.h>

void mtx_init(struct mtx *m, const char *name, const char *type, int opts)
{
    mutex_init(&m->_mutex);
}

void mtx_destroy(struct mtx *m)
{
    mutex_destroy(&m->_mutex);
}

void mtx_lock(struct mtx *mp)
{
    mutex_lock(&mp->_mutex);
}

int mtx_trylock(struct mtx *mp)
{
    return mutex_trylock(&mp->_mutex) ? 1 : 0;
}

void mtx_unlock(struct mtx *mp)
{
    mutex_unlock(&mp->_mutex);
}

void mtx_assert(struct mtx *mp, int flag)
{
    switch (flag) {
    case MA_OWNED:
        if (!mutex_owned(&mp->_mutex)) {
            abort();
        }
        break;
    case MA_NOTOWNED:
        if (mutex_owned(&mp->_mutex)) {
            abort();
        }
        break;
    default:
        abort();
    }
}

void sx_init(struct sx *s, const char *name)
{
    rwlock_init(&s->_rw);
}

void sx_destroy(struct sx *s)
{
    rwlock_destroy(&s->_rw);
}

void sx_xlock(struct sx *s)
{
    rw_wlock(&s->_rw);
}

void sx_xunlock(struct sx *s)
{
    rw_wunlock(&s->_rw);
}

void sx_slock(struct sx *s)
{
    rw_rlock(&s->_rw);
}

void sx_sunlock(struct sx *s)
{
    rw_runlock(&s->_rw);
}
