
#ifndef RTE_PMD_MLX5_CUSTOM_H
#define RTE_PMD_MLX5_CUSTOM_H

void *mlx5_manual_reg_mr(uint8_t port_id, void *addr, size_t length, uint32_t *lkey_out);
void mlx5_manual_dereg_mr(void *ibv_mr);

#endif /* RTE_PMD_MLX5_CUSTOM_H */
