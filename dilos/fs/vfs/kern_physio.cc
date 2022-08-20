/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <new>

#include <osv/device.h>
#include <osv/bio.h>
#include <sys/param.h>
#include <assert.h>
#include <sys/refcount.h>
#include <osv/mutex.h>
#include <osv/waitqueue.hh>

struct bio *
alloc_bio(void)
{
	auto *b = new (std::nothrow) bio();
	if (!b)
		return nullptr;
	return b;
}

void
destroy_bio(struct bio *bio)
{
	delete bio;
}

int
bio_wait(struct bio *bio)
{
	SCOPE_LOCK(bio->bio_mutex);
	while (!(bio->bio_flags & BIO_DONE)) {
		bio->bio_wait.wait(bio->bio_mutex);
	}
	if (bio->bio_flags & BIO_ERROR) {
		return EIO;
	}
	return 0;
}

void
biodone(struct bio *bio, bool ok)
{
	WITH_LOCK(bio->bio_mutex) {
		bio->bio_flags |= BIO_DONE;
		if (!ok)
			bio->bio_flags |= BIO_ERROR;
		if (!bio->bio_done) {
			bio->bio_wait.wake_one(bio->bio_mutex);
			return;
		}
	}
	bio->bio_done(bio);
}

void
biofinish(struct bio *bp, struct devstat *stat, int error)
{
	if (error) {
		bp->bio_error = error;
	}
	biodone(bp, error);
}

static void multiplex_bio_done(struct bio *b)
{
	struct bio *bio = static_cast<struct bio*>(b->bio_caller1);
	bool error = b->bio_flags & BIO_ERROR;
	destroy_bio(b);


	// If there is an error, we store it in the original bio flags.
	// This path gets slower because then we need to end up taking the
	// bio_mutex twice. But that should be fine.
	if (error) {
		WITH_LOCK(bio->bio_mutex) {
			bio->bio_flags |= BIO_ERROR;
		}
	}

	// Last one releases it. We set the biodone to always be "ok", because
	// if an error exists, we have already set that in the previous operation
	if (refcount_release(&bio->bio_refcnt))
		biodone(bio, true);
}

void multiplex_strategy(struct bio *bio)
{
	struct device *dev = bio->bio_dev;
	devop_strategy_t strategy = *((devop_strategy_t *)dev->private_data);

	uint64_t len = bio->bio_bcount;

	bio->bio_offset += bio->bio_dev->offset;
	uint64_t offset = bio->bio_offset;
	void *buf = bio->bio_data;

	assert(strategy != nullptr);

	if (len <= dev->max_io_size) {
		strategy(bio);
		return;
	}

	// It is better to initialize the refcounter beforehand, specially because we can
	// trivially determine what is the number going to be. Otherwise, we can have a
	// situation in which we bump the refcount to 1, get scheduled out, the bio is
	// finished, and when it drops its refcount to 0, we consider the main bio finished.
	refcount_init(&bio->bio_refcnt, (len / dev->max_io_size) + !!(len % dev->max_io_size));

	while (len > 0) {
		uint64_t req_size = MIN(len, dev->max_io_size);
		struct bio *b = alloc_bio();

		b->bio_bcount = req_size;
		b->bio_data = buf;
		b->bio_offset = offset;

		b->bio_cmd = bio->bio_cmd;
		b->bio_dev = bio->bio_dev;
		b->bio_caller1 = bio;
		b->bio_private = bio->bio_private;
		b->bio_done = multiplex_bio_done;

		strategy(b);
		buf += req_size;
		offset += req_size;
		len -= req_size;
	}
}
