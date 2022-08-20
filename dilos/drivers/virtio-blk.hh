/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef VIRTIO_BLK_DRIVER_H
#define VIRTIO_BLK_DRIVER_H
#include "drivers/virtio.hh"
#include "drivers/virtio-device.hh"
#include <osv/bio.h>

namespace virtio {

class blk : public virtio_driver {
public:

    // The feature bitmap for virtio blk
    enum {
        VIRTIO_BLK_F_BARRIER    = 0,  /* Does host support barriers? */
        VIRTIO_BLK_F_SIZE_MAX   = 1,  /* Indicates maximum segment size */
        VIRTIO_BLK_F_SEG_MAX    = 2,  /* Indicates maximum # of segments */
        VIRTIO_BLK_F_GEOMETRY   = 4,  /* Legacy geometry available  */
        VIRTIO_BLK_F_RO         = 5,  /* Disk is read-only */
        VIRTIO_BLK_F_BLK_SIZE   = 6,  /* Block size of disk is available*/
        VIRTIO_BLK_F_SCSI       = 7,  /* Supports scsi command passthru */
        VIRTIO_BLK_F_WCE        = 9,  /* Writeback mode enabled after reset */
        VIRTIO_BLK_F_TOPOLOGY   = 10, /* Topology information is available */
        VIRTIO_BLK_F_CONFIG_WCE = 11, /* Writeback mode available in config */
    };

    enum {
        VIRTIO_BLK_ID_BYTES = 20, /* ID string length */

        /*
         * Command types
         *
         * Usage is a bit tricky as some bits are used as flags and some are not.
         *
         * Rules:
         *   VIRTIO_BLK_T_OUT may be combined with VIRTIO_BLK_T_SCSI_CMD or
         *   VIRTIO_BLK_T_BARRIER.  VIRTIO_BLK_T_FLUSH is a command of its own
         *   and may not be combined with any of the other flags.
         */
    };

    enum blk_request_type {
        VIRTIO_BLK_T_IN = 0,
        VIRTIO_BLK_T_OUT = 1,
        /* This bit says it's a scsi command, not an actual read or write. */
        VIRTIO_BLK_T_SCSI_CMD = 2,
        /* Cache flush command */
        VIRTIO_BLK_T_FLUSH = 4,
        /* Get device ID command */
        VIRTIO_BLK_T_GET_ID = 8,
        /* Barrier before this op. */
        VIRTIO_BLK_T_BARRIER = 0x80000000,
    };

    enum blk_res_code {
        /* And this is the final byte of the write scatter-gather list. */
        VIRTIO_BLK_S_OK = 0,
        VIRTIO_BLK_S_IOERR = 1,
        VIRTIO_BLK_S_UNSUPP = 2,
    };

    struct blk_config {
            /* The capacity (in 512-byte sectors). */
            u64 capacity;
            /* The maximum segment size (if VIRTIO_BLK_F_SIZE_MAX) */
            u32 size_max;
            /* The maximum number of segments (if VIRTIO_BLK_F_SEG_MAX) */
            u32 seg_max;
            /* geometry the device (if VIRTIO_BLK_F_GEOMETRY) */
            struct blk_geometry {
                    u16 cylinders;
                    u8 heads;
                    u8 sectors;
            } geometry;

            /* block size of device (if VIRTIO_BLK_F_BLK_SIZE) */
            u32 blk_size;

            struct blk_topology {
                    /* the next 4 entries are guarded by VIRTIO_BLK_F_TOPOLOGY  */
                    /* exponent for physical block per logical block. */
                    u8 physical_block_exp;
                    /* alignment offset in logical blocks. */
                    u8 alignment_offset;
                    /* minimum I/O size without performance penalty in logical blocks. */
                    u16 min_io_size;
                    /* optimal sustained I/O size in logical blocks. */
                    u32 opt_io_size;
            } topology;

            /* writeback mode (if VIRTIO_BLK_F_CONFIG_WCE) */
            u8 wce;
    } __attribute__((packed));

    /* This is the first element of the read scatter-gather list. */
    struct blk_outhdr {
            /* VIRTIO_BLK_T* */
            u32 type;
            /* io priority. */
            u32 ioprio;
            /* Sector (ie. 512 byte offset) */
            u64 sector;
    };

    struct virtio_scsi_inhdr {
            u32 errors;
            u32 data_len;
            u32 sense_len;
            u32 residual;
    };

    struct blk_res {
        u8 status;
    };

    explicit blk(virtio_device& dev);
    virtual ~blk();

    virtual std::string get_name() const { return _driver_name; }
    void read_config();

    virtual u32 get_driver_features();

    int make_request(struct bio*);

    void req_done();
    int64_t size();

    void set_readonly() {_ro = true;}
    bool is_readonly() {return _ro;}

    bool ack_irq();

    static hw_driver* probe(hw_device* dev);
private:

    struct blk_req {
        blk_req(struct bio* b) :bio(b) {};
        ~blk_req() {};

        blk_outhdr hdr;
        blk_res res;
        struct bio* bio;
    };

    std::string _driver_name;
    blk_config _config;

    //maintains the virtio instance number for multiple drives
    static int _instance;
    int _id;
    bool _ro;
    // This mutex protects parallel make_request invocations
    mutex _lock;
};

}
#endif

