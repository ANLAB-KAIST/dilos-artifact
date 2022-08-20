#!/bin/bash
source config.sh

echo "Setup Bridge"
brctl addbr dilosbr
brctl addif dilosbr ${ETH_IF}
brctl show
ip addr add ${GW}/${PREFIX} dev dilosbr
ip link set ${ETH_IF} up
ip link set dilosbr up
sysctl -w net.ipv4.ip_forward=1

echo "Setup RDMA"
ip addr add ${RDMA_IP}/${PREFIX} dev ${RDMA_IF}
ip link set ${RDMA_IF} up
