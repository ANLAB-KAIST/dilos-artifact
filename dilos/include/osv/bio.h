/*-
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)buf.h	8.9 (Berkeley) 3/30/95
 * $FreeBSD$
 */

#ifndef _SYS_BIO_H_
#define	_SYS_BIO_H_

#include <sys/cdefs.h>
#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include <bsd/sys/sys/queue.h>
#include <osv/device.h>
#include <osv/mutex.h>
#include <osv/waitqueue.hh>

__BEGIN_DECLS

/* bio_cmd */
#define BIO_READ	0x01
#define BIO_WRITE	0x02
#define BIO_DELETE	0x04
#define BIO_GETATTR	0x08
#define BIO_FLUSH	0x10
#define BIO_SCSI	0x20
#define BIO_CMD1	0x40	/* Available for local hacks */
#define BIO_CMD2	0x80	/* Available for local hacks */

/* bio_flags */
#define BIO_ERROR	0x01
#define BIO_DONE	0x02
#define BIO_ONQUEUE	0x04
#define BIO_ORDERED	0x08

struct disk;
struct bio;

/* Empty classifier tag, to prevent further classification. */
#define	BIO_NOTCLASSIFIED		(void *)(~0UL)

typedef void bio_task_t(void *);
typedef uint64_t   daddr_t;    /* disk address */


/*
 * The bio structure describes an I/O operation in the kernel.
 */
struct bio {
	uint8_t	bio_cmd;		/* I/O operation. */
	uint8_t	bio_flags;		/* General flags. */
	struct device *bio_dev;		/* Device to do I/O on. */
	off_t	bio_offset;		/* Offset into file. */
	size_t	bio_bcount;		/* Valid bytes in buffer. */
	void	*bio_data;		/* Memory, superblocks, indirect etc. */
	int	bio_error;		/* Errno for BIO_ERROR. */
	long	bio_resid;		/* Remaining I/O in bytes. */
	void	*bio_caller1;		/* Private use by the consumer. */
	void	*bio_private;		/* Private use low-level device driver. */
	void	(*bio_done)(struct bio *);
	struct disk *bio_disk;
	daddr_t bio_pblkno;
	off_t   bio_length;     /* Like bio_bcount */

	TAILQ_ENTRY(bio) bio_queue;

	/*
	 * I/O synchronization, probably should move out of the struct to
	 * save space.
	 */
	mutex_t bio_mutex;
	waitqueue bio_wait;
	volatile unsigned int bio_refcnt;
};

struct bio_queue_head {
	TAILQ_HEAD(bio_queue, bio) queue;
	off_t last_offset;
	struct	bio *insert_point;
};
struct bio *bioq_takefirst(struct bio_queue_head *head);
struct bio *bioq_first(struct bio_queue_head *head);

void bioq_init(struct bio_queue_head *head);
void bioq_insert_tail(struct bio_queue_head *head, struct bio *bp);
void bioq_insert_head(struct bio_queue_head *head, struct bio *bp);
void bioq_remove(struct bio_queue_head *head, struct bio *bp);

struct bio *	alloc_bio(void);
void		destroy_bio(struct bio *bio);

int		bio_wait(struct bio *bio);
void		biodone(struct bio *bio, bool ok);
struct devstat;
void    biofinish(struct bio *bp, struct devstat *stat, int error);

__END_DECLS

#endif /* !_SYS_BIO_H_ */
