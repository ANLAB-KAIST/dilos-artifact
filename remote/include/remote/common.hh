#pragma once
#include <infiniband/verbs.h>

#include <cstring>
#include <string>
#include <vector>

namespace remote {

namespace cfg {

constexpr enum ibv_mtu mtu = IBV_MTU_1024;
constexpr uint16_t pkey_index = 0;
constexpr uint32_t rq_psn = 0;
constexpr uint8_t max_dest_rd_atomic = 16;
constexpr uint8_t min_rnr_timer = 12;
constexpr uint8_t sl = 0;
constexpr uint8_t src_path_bits = 0;

constexpr uint32_t flow_label = 0;
constexpr uint8_t hop_limit = 1;
constexpr uint8_t traffic_class = 0;

constexpr uint8_t timeout = 14;
constexpr uint8_t retry_cnt = 7;
constexpr uint8_t rnr_retry = 7;

constexpr uint32_t sq_psn = 0;
constexpr uint8_t max_rd_atomic = 16;

namespace server {
constexpr int mr_flags =
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE;
constexpr int qp_init_flags =
    IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS;
constexpr int qp_rtr_flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
                             IBV_QP_DEST_QPN | IBV_QP_RQ_PSN |
                             IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;
constexpr int qp_rts_flags = IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
                             IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN |
                             IBV_QP_MAX_QP_RD_ATOMIC;
constexpr int qp_access_flags =
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE;

}  // namespace server

namespace client {

constexpr int max_sge = 30;

constexpr int mr_flags = IBV_ACCESS_LOCAL_WRITE;

constexpr int qp_access_flags =
    IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_WRITE;
constexpr int qp_init_flags =
    IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT | IBV_QP_ACCESS_FLAGS;
constexpr int qp_rtr_flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU |
                             IBV_QP_DEST_QPN | IBV_QP_RQ_PSN |
                             IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;
constexpr int qp_rts_flags = IBV_QP_STATE | IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT |
                             IBV_QP_RNR_RETRY | IBV_QP_SQ_PSN |
                             IBV_QP_MAX_QP_RD_ATOMIC;
}  // namespace client
}  // namespace cfg
struct ctx_t {
    struct ibv_device_attr device_attr;
    struct ibv_context *ib_ctx = NULL;
    struct ibv_port_attr port_attr;
    int port_id;
    union ibv_gid gid;
    int gid_idx;
    uint16_t lid;
};

struct peer_t {
    int peer_sock;  // tcp
    struct ibv_pd *pd;
    uint8_t gid[16];
    uint16_t lid;
};

void init_ib(struct ctx_t &ctx, const std::string ib_device, const int ib_port,
             const int gid_idx) {
    int num_devices;
    struct ibv_device *ib_dev = NULL;
    struct ibv_device **dev_list = ibv_get_device_list(&num_devices);

    if (!dev_list) {
        printf("no rdma devices");
        abort();
    }

    if (!num_devices) {
        printf("zero rdma devices");
        abort();
    }

    for (int i = 0; i < num_devices; i++) {
        if (!strcmp(ibv_get_device_name(dev_list[i]), ib_device.c_str())) {
            ib_dev = dev_list[i];
            break;
        }
    }
    if (!ib_dev) {
        printf("no device: %s", ib_device.c_str());
        abort();
    }

    struct ibv_context *ib_ctx = NULL;

    ib_ctx = ibv_open_device(ib_dev);
    if (!ib_ctx) {
        printf("fail to open device");
        abort();
    }

    struct ibv_device_attr device_attr;
    memset(&device_attr, 0, sizeof(device_attr));

    int ret = ibv_query_device(ib_ctx, &device_attr);
    if (ret != 0) {
        printf("fail to query device");
        abort();
    }
    ibv_free_device_list(dev_list);
    dev_list = NULL;
    ib_dev = NULL;

    struct ibv_port_attr port_attr;
    memset(&port_attr, 0, sizeof(port_attr));
    if (ibv_query_port(ib_ctx, ib_port, &port_attr)) {
        printf("fail to query port");
        abort();
    }

    union ibv_gid gid;
    if (ibv_query_gid(ib_ctx, ib_port, gid_idx, &gid)) {
        printf("fail to query gid");
        abort();
    }
    memcpy(&ctx.device_attr, &device_attr, sizeof(device_attr));
    ctx.ib_ctx = ib_ctx;
    memcpy(&ctx.port_attr, &port_attr, sizeof(port_attr));
    ctx.port_id = ib_port;
    memcpy(&ctx.gid, &gid, sizeof(gid));
    ctx.gid_idx = gid_idx;
    ctx.lid = port_attr.lid;
}

}  // namespace remote
