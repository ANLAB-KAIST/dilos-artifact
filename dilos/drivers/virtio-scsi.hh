/*
 * Copyright (C) 2013 Cloudius Systems, Ltd.
 *
 * This work is open source software, licensed under the terms of the
 * BSD license as described in the LICENSE file in the top-level directory.
 */

#ifndef VIRTIO_SCSI_DRIVER_H
#define VIRTIO_SCSI_DRIVER_H
#include "drivers/virtio.hh"
#include "drivers/scsi-common.hh"
#include <osv/bio.h>
#include <osv/types.h>

namespace virtio {

class scsi : public virtio_driver, public scsi_common {
public:
    // The feature bitmap for virtio scsi
    enum {
        VIRTIO_SCSI_F_INOUT = 0,
        VIRTIO_SCSI_F_HOTPLUG = 1,
        VIRTIO_SCSI_F_CHANGE = 2,
    };

    enum {
        VIRTIO_SCSI_QUEUE_CTRL = 0,
        VIRTIO_SCSI_QUEUE_EVT = 1,
        VIRTIO_SCSI_QUEUE_REQ = 2,
    };

    enum {
        VIRTIO_SCSI_CDB_SIZE = 32,
        VIRTIO_SCSI_SENSE_SIZE = 96,
        VIRTIO_SCSI_SECTOR_SIZE = 512,
    };

    enum scsi_res_code {
        /* And this is the final byte of the write scatter-gather list. */
        VIRTIO_SCSI_S_OK                      =  0,
        VIRTIO_SCSI_S_OVERRUN                 =  1,
        VIRTIO_SCSI_S_ABORTED                 =  2,
        VIRTIO_SCSI_S_BAD_TARGET              =  3,
        VIRTIO_SCSI_S_RESET                   =  4,
        VIRTIO_SCSI_S_BUSY                    =  5,
        VIRTIO_SCSI_S_TRANSPORT_FAILURE       =  6,
        VIRTIO_SCSI_S_TARGET_FAILURE          =  7,
        VIRTIO_SCSI_S_NEXUS_FAILURE           =  8,
        VIRTIO_SCSI_S_FAILURE                 =  9,
        VIRTIO_SCSI_S_FUNCTION_SUCCEEDED      =  10,
        VIRTIO_SCSI_S_FUNCTION_REJECTED       =  11,
        VIRTIO_SCSI_S_INCORRECT_LUN           =  12,
    };

    struct scsi_config {
        u32 num_queues;
        u32 seg_max;
        u32 max_sectors;
        u32 cmd_per_lun;
        u32 event_info_size;
        u32 sense_size;
        u32 cdb_size;
        u16 max_channel;
        u16 max_target;
        u32 max_lun;
    } __attribute__((packed));

    /* SCSI command request, followed by data-out */
    struct scsi_cmd_req {
        u8 lun[8];		/* Logical Unit Number */
        u64 tag;		/* Command identifier */
        u8 task_attr;   /* Task attribute */
        u8 prio;
        u8 crn;
        u8 cdb[VIRTIO_SCSI_CDB_SIZE];
    } __attribute__((packed));

    /* Response, followed by sense data and data-in */
    struct scsi_cmd_resp {
        u32 sense_len;		/* Sense data length */
        u32 resid;		/* Residual bytes in data buffer */
        u16 status_qualifier;	/* Status qualifier */
        u8 status;		/* Command completion status */
        u8 response;		/* Response values */
        u8 sense[VIRTIO_SCSI_SENSE_SIZE];
    } __attribute__((packed));

    /* Task Management Request */
    struct scsi_ctrl_tmf_req {
        u32 type;
        u32 subtype;
        u8 lun[8];
        u64 tag;
    } __attribute__((packed));

    struct scsi_ctrl_tmf_resp {
        u8 response;
    } __attribute__((packed));

    /* Asynchronous notification query/subscription */
    struct scsi_ctrl_an_req {
        u32 type;
        u8 lun[8];
        u32 event_requested;
    } __attribute__((packed));

    struct scsi_ctrl_an_resp {
        u32 event_actual;
        u8 response;
    } __attribute__((packed));

    struct scsi_event {
        u32 event;
        u8 lun[8];
        u32 reason;
    } __attribute__((packed));

    class scsi_virtio_req: public scsi_common_req {
    public:
        scsi_virtio_req(struct bio *bio, u16 target, u16 lun, u8 cmd)
            : scsi_common_req(bio, target, lun, cmd)
        {
            memset(&req.cmd, 0, sizeof(req.cmd));
            req.cmd.tag = reinterpret_cast<u64>(bio);
            req.cmd.lun[0] = 1;
            req.cmd.lun[1] = target;
            req.cmd.lun[2] = (lun >> 8) | 0x40;
            req.cmd.lun[3] = (lun & 0xff);
        }

        union {
            struct scsi_cmd_req       cmd;
            struct scsi_ctrl_tmf_req  tmf;
            struct scsi_ctrl_an_req   an;
        } req;

        union {
            struct scsi_cmd_resp      cmd;
            struct scsi_ctrl_tmf_resp tmf;
            struct scsi_ctrl_an_resp  an;
            struct scsi_event         evt;
        } resp;
    };


    scsi(virtio_device& dev);
    ~scsi();

    virtual std::string get_name() const { return _driver_name; }
    void read_config();

    virtual u32 get_driver_features();

    static struct scsi_priv *get_priv(struct bio *bio) {
        return reinterpret_cast<struct scsi_priv*>(bio->bio_dev->private_data);
    }

    void req_done();
    bool ack_irq();
    static hw_driver* probe(hw_device* dev);

    virtual int make_request(struct bio*) override;
    virtual void add_lun(u16 target_id, u16 lun_id) override;
    virtual int exec_cmd(struct bio *bio) override;
    virtual scsi_virtio_req *alloc_scsi_req(struct bio *bio, u16 target, u16 lun, u8 cmd) override
    {
        return new scsi_virtio_req(bio, target, lun, cmd);
    }

private:

    std::string _driver_name;
    scsi_config _config;

    //maintains the virtio instance number for multiple drives
    static int _instance;
    int _id;

    // This mutex protects parallel make_request invocations
    mutex _lock;
};
}
#endif
