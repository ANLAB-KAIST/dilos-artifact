# DiLOS - Fastswap Evaluation

## 1. Software Setup Instruction

**WARNING: Takes long time**

Note: Install Ubuntu 18.04 LTS and extract the artifact on `/root/dilos-fastswap` on both nodes.

Run below on both **compute and memory nodes**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./install.deps.sh # 3. Install dependencies
./install.kernel.fastswap.sh # 4. Install fastswap's kernel
reboot # 5. Reboot
# Note: reboot to fastswap's kernel
./install.ofed.sh # 6. Install OFED
reboot # 7. Reboot
# Note: reboot to fastswap's kernel
./setup.lxc.sh # 8. Setup an LXC container
```

## 2. Dataset Download

**WARNING: Takes long time**

```bash
./download.dataset.sh # Download dataset in container's /mnt
```

## 3. Build Apps for Experiment

```bash
./build.apps.fastswap.sh # build apps
```

## 4. Experiment Configuration (You must start here after reboot)

1. Modify `config.sh`, if you need.

2. Run below on **memory node**.

**This script must run before compute node's script.**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./setup.remote.fastswap.sh # 3. Do configuration
```

3. Run below on **compute node**

**This script must run after memory node's script.**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./setup.compute.fastswap.sh # 3. Do configuration
```

## 5. Experiments

* Change `config.sh`, if you need.
  * `TRIES=1` : Number of tries
* All results are stored in `/root/benchmark-out`

### 5.1. **Figure 7(a)**: Quicksort

**WARNING: Takes long time**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./scripts/bench.quicksort.fastswap.sh # 3. Fire
```

### 5.2. **Figure 7(b)**: K-Means

**WARNING: Takes long time**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./scripts/bench.kmeans.fastswap.sh # 3. Fire
```

### 5.3. **Figure 7(c) & 7(d)**: Compression & Decompression

**WARNING: Takes long time**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./scripts/bench.quicksort.fastswap.sh # 3. Fire
```

### 5.5. **Figure 8**: Dataframe

**WARNING: Takes long time**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./scripts/bench.dataframe.fastswap.sh # 3. Fire
```

### 5.6. **Figure 9**: GAPBS

**WARNING: Takes long time**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./scripts/bench.gapbs.fastswap.sh # 3. Fire
```

### 5.7. **Figure 10**: Redis

**WARNING: Takes long time**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos-fastswap # 2. Change directory
./scripts/bench.redis.fastswap.sh # 3. Fire
```
