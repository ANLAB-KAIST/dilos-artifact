#ifndef LIBC_H
#define LIBC_H

#include <stdlib.h>
#include <osv/mutex.h>
#include "stdio.h"

__BEGIN_DECLS

/* as long as we use the glibc header we'll need this hack */
#ifndef O_LARGEFILE
#define O_LARGEFILE	0
#endif

struct __libc {
	void *main_thread;
//	int threaded;
//	int secure;
//	size_t *auxv;
//	int (*atexit)(void (*)(void));
//	void (*fini)(void);
//	void (*ldso_fini)(void);
//	volatile int threads_minus_1;
//	int canceldisable;
	FILE *ofl_head;
	mutex_t ofl_lock;
//	size_t tls_size;
};

#define ATTR_LIBC_VISIBILITY __attribute__((visibility("hidden")))

extern struct __libc __libc ATTR_LIBC_VISIBILITY;
#define libc __libc

int __lockfile(FILE *) ATTR_LIBC_VISIBILITY;
void __unlockfile(FILE *) ATTR_LIBC_VISIBILITY;
#define LOCK(x) mutex_lock(&(x))
#define UNLOCK(x) mutex_unlock(&(x))

extern char **__environ;

#undef weak_alias
#define __weak_alias(old, new) \
	__typeof(old) new __attribute__((weak, alias(#old)))
#ifdef __cplusplus
#define weak_alias(old, new) extern "C" __weak_alias(old, new)
#else
#define weak_alias(old, new) extern __weak_alias(old, new)
#endif

#undef LFS64_2
#define LFS64_2(x, y) weak_alias(x, y)

#undef LFS64
#define LFS64(x) LFS64_2(x, x##64)

extern void _chk_fail(const char *func);
__END_DECLS

#endif
