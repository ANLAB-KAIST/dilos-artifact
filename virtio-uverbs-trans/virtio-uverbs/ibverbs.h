#ifndef VIRTIO_UVERBS_TRANS_IBVERBS_H
#define VIRTIO_UVERBS_TRANS_IBVERBS_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef LIBUVERBS_UPDATE_ATTR
#error LIBUVERBS_UPDATE_ATTR not found
#endif

static int ibverbs_translate_general(uint32_t command, char *buff) {
    switch (command) {
        case IB_USER_VERBS_CMD_REG_MR: {
            struct ibv_reg_mr *request = (struct ibv_reg_mr *)buff;
            LIBUVERBS_UPDATE_ATTR(request, start);
            // LIBUVERBS_UPDATE_ATTR(request, hca_va);
            break;
        }
        case IB_USER_VERBS_CMD_CREATE_CQ: {
            struct ibv_create_cq *request = (struct ibv_create_cq *)buff;
            LIBUVERBS_UPDATE_ATTR(request, user_handle);
            break;
        }
        case IB_USER_VERBS_CMD_CREATE_QP: {
            struct ibv_create_qp *request = (struct ibv_create_qp *)buff;
            LIBUVERBS_UPDATE_ATTR(request, user_handle);
            break;
        }
        default:
            break;
    }

    return 0;
}

static int ibverbs_translate_exp(uint32_t command, char *buff) {
    abort();
    return 0;
}

static int ibverbs_translate_ext(uint32_t command, char *buff) { return 0; }

#ifdef __cplusplus
}
#endif
#endif