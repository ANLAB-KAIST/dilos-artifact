### ROOT Path

ROOT_PATH=$(dirname "$(realpath "${BASH_SOURCE[0]}")")
cd $ROOT_PATH || exit

### Compute Node Configuration ###

BUILD_TYPE=RelWithDebInfo
NO_SG=${NO_SG:=n}

# Network Configuration

ETH_IF=eno2
IP=172.16.0.2
SUBNET=255.255.255.0
PREFIX=24
GW=172.16.0.1
NAMESERVER=8.8.8.8
TAP_NAME=pusnow_tap0
RDMA_IF=ens2f0
RDMA_IP=192.168.123.2

# CPU Configuration

HYPERVISOR=qemu_microvm
NODE=0
NODE_CPUS=0-11
CPUS=${CPUS:=8}
MEMORY=${MEMORY:=40G}
FS=rofs
PCI=device
PREFETCHER=${PREFETCHER}
TRACE=
#ddc_remote*
#ddc_evict*,ddc_mmu_fault_*
#ddc_mmu_*
#ddc_mmu_fault_*,ddc_evict_*
#ddc_mmu_prefetch_remote_offset,ddc_mmu_prefetch_remote_offset_page,ddc_mmu_polled*
#ddc_evict_\*
#,ddc_mmu_fault_remote_page_done\*,ddc_eviction_\*
AUTO_POWEROFF=${AUTO_POWEROFF:=n}

# Remote Configuration

REMOTE_DRIVER=${REMOTE_DRIVER:=rdma}
REMOTE_LOCAL_SIZE_GB=32

if [[ "$REMOTE_DRIVER" == "local" ]]; then
    MEM_EXTRA_MB=$(expr ${REMOTE_LOCAL_SIZE_GB} \* 1024)
else
    MEM_EXTRA_MB=0
fi

# Allocator Configuration
MIMALLOC=${MIMALLOC:=mimalloc}
#-subpage

# Infiniband Configuration

IB_DEVICE=mlx5_0
IB_PORT=1
GID_IDX=1

### Compute Node Configuration (END) ###

### Memory Node Configuration ###
MS_ETH_IF=eno2
MS_IP=172.16.0.3
MS_PORT=12345
MS_PATH="~/dilos"
MS_NODE=1
MS_RDMA_IF=ens2f0
MS_RDMA_IP=192.168.123.3
MS_MEMORY_GB=${MS_MEMORY_GB:=40}

### Memory Node Configuration (END) ###

### Benchmark Configuration ###

OUT_PATH=$HOME/benchmark-out/
DATE=$(date +%Y-%m-%d_%H-%M-%S)

TWITTER=/mnt/twitter/twitter.sg
TWITTERU=/mnt/twitter/twitterU.sg
TRIP_DATA_CSV=/mnt/tripdata/all.csv
ENWIKI_UNCOMP=/mnt/enwiki/enwik9.uncompressed
ENWIKI_COMP=/mnt/enwiki/enwik9.compressed

### Benchmark Configuration (END) ###
