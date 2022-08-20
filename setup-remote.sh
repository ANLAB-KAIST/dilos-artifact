#!/bin/bash
source config.sh

echo "Setup IP"
ip addr add ${MS_IP}/${PREFIX} dev ${MS_ETH_IF}
ip link set ${MS_ETH_IF} up

echo "Setup HugeTLB"
hugeadm --pool-pages-min 2M:300G
hugeadm --create-mounts

echo "Setup RDMA"
ip addr add ${MS_RDMA_IP}/${PREFIX} dev ${MS_RDMA_IF}
ip link set ${MS_RDMA_IF} up
