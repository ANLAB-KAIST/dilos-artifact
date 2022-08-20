#ifndef VIRTIO_UVERBS_TRANS_MLX5_H
#define VIRTIO_UVERBS_TRANS_MLX5_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#ifndef LIBUVERBS_UPDATE_ATTR
#error LIBUVERBS_UPDATE_ATTR not found
#endif

static int mlx5_translate_general(uint32_t command, char *buff) {
    switch (command) {
        case IB_USER_VERBS_CMD_CREATE_CQ: {
            struct mlx5_ib_create_cq *request =
                (struct mlx5_ib_create_cq *)(buff +
                                             sizeof(struct ibv_create_cq));
            LIBUVERBS_UPDATE_ATTR(request, buf_addr);
            LIBUVERBS_UPDATE_ATTR(request, db_addr);
            break;
        }
        case IB_USER_VERBS_CMD_CREATE_QP: {
            struct mlx5_ib_create_qp *request =
                (struct mlx5_ib_create_qp *)(buff +
                                             sizeof(struct ibv_create_qp));
            LIBUVERBS_UPDATE_ATTR(request, buf_addr);
            LIBUVERBS_UPDATE_ATTR(request, db_addr);

            break;
        }
        default:
            break;
    }

    return 0;
}

static int mlx5_translate_ext(uint32_t command, char *buff) { return 0; }

static int mlx5_translate_exp(uint32_t command, char *buff) {
    abort();

    return 0;
}

#ifdef __cplusplus
}
#endif
#endif