#!/bin/bash
source config.sh

QEMU_PATH=build/qemu

source config.sh

if [[ -n $TRACE ]]; then
    TRACE_ARGS="--trace=${TRACE}"
fi

if [[ -n $NODE ]]; then
    if [[ -n $NODE_CPUS ]]; then
        NUMA_ARG="--numa ${NODE} --numa_cpus ${NODE_CPUS}"
    else
        NUMA_ARG="--numa ${NODE}"
    fi
fi

if [[ -n $VERBOSE ]]; then
    VERBOSE_ARG="--verbose"
fi
if [[ -z $NO_NETWORK ]]; then
    NETWORK_ARGS="-n -b dilosbr --vhost"
fi

if [[ -n $DISK ]]; then
    DISK_HD=hd1
    DISK_BLK=vblk1
    DISK_IMG_OPT="--pass-args=-device --pass-args=virtio-blk-${PCI},id=${DISK_BLK},drive=${DISK_HD},scsi=off --pass-args=-drive --pass-args=file=${DISK},if=none,id=${DISK_HD},cache=none,aio=native "
    DISK_MOUNT_CMD="--mount-fs=rofs,/dev/${DISK_BLK},/mnt"
fi

RUN_ARGS="${VERBOSE_ARG} \
--qemu-path ${QEMU_PATH}/x86_64-softmmu/qemu-system-x86_64 \
-p ${HYPERVISOR} \
-c ${CPUS} \
-m ${MEMORY} \
${NUMA_ARG} \
${NETWORK_ARGS} \
--pass-args=-device --pass-args=virtio-uverbs-${PCI},host=uverbs0 \
${DISK_IMG_OPT} \
${TRACE_ARGS} "

IB_CMD="--ib_device=${IB_DEVICE} --ib_port=${IB_PORT} --ms_ip=${MS_IP} --ms_port=${MS_PORT} --gid_idx=${GID_IDX}"
NETWORK_CMD="--ip=eth0,${IP},${SUBNET} --defaultgw=${GW} --nameserver=${NAMESERVER}"

if [[ -n "${PREFETCHER}" ]]; then
    PREFETCHER_CMD="--prefetcher=${PREFETCHER}"
fi

if [[ "${AUTO_POWEROFF}" = "y" ]]; then
    POWEROFF_CMD="--power-off-on-abort"
else
    POWEROFF_CMD="--noshutdown"
fi

if [[ -n "${PIN}" ]]; then
    PIN_CMD="--cpu_pin=${PIN}"
fi

CMD_PREFIX="${NETWORK_CMD} ${IB_CMD} ${PREFETCHER_CMD} ${DISK_MOUNT_CMD} ${POWEROFF_CMD} ${PIN_CMD} --disable_rofs_cache "

if [[ -n "$*" ]]; then
    CMDLINE="$*"
else
    CMDLINE=$(cat dilos/build/last/cmdline)
fi

echo $RUN_ARGS
echo $CMD_PREFIX

export QEMU_NBD=${ROOT_PATH}/${QEMU_PATH}/qemu-nbd
export QEMU_IMG=${ROOT_PATH}/${QEMU_PATH}/qemu-img

./dilos/scripts/run.py ${RUN_ARGS} -e "${CMD_PREFIX} ${CMDLINE}"
