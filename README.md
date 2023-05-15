# DiLOS - Artifact

Paper: [https://wsyo.one/dilos](https://wsyo.one/dilos)  
Slides: [https://wsyo.one/dilos/slides](https://wsyo.one/dilos/slides)



**Note: EagleOS is an anonymized code name of DiLOS**

## 1. Software Setup Instruction

Note: Install Ubuntu 18.04 LTS and extract the artifact on `/root/dilos` on both nodes.

Run below on both **compute and memory nodes**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./install-deps.sh # 3. Install dependencies
./install-ofed.sh # 4. Install OFED
reboot # 5. Reboot
```

## 2. Build (Optional, Automatic Rebuild on Experiement)

Run below on both **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./build.sh # 3. Build
```

## 3. Experiment Preperation

### 3.1. Disk Setup

**WARNING: Takes long time**

Run below on **compute node**.

```bash
./scripts/download-dataset.sh # Download dataset in /mnt
./scripts/prepare-disk.sh # Generated disk images contain dataset
```

### 3.2. Build Redis Benchmark Tool

Run below on **compute node**.

```bash
./scripts/prepare-redis.sh # build redis-benchmark
```

## 4. Experiment Configuration (You must start here after reboot)

1. Modify `config.sh`, if you need.

2. Run below on **compute node**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./setup-compute.sh # 3. Do configuration
```

3. Run below on **memory node**

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./setup-remote.sh # 3. Do configuration
```

## 5. Experiments

* Change `scripts/config-bench.sh`, if you need.
  * `TRIES=1` : Number of tries
* All results are stored in `/root/benchmark-out`

### 5.1. **Figure 7(a)**: Quicksort

**WARNING: Takes long time**

Run below on **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./scripts/bench-quicksort.sh # 3. Fire
```

### 5.2. **Figure 7(b)**: K-Means

**WARNING: Takes long time**

Run below on **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./scripts/bench-kmeans.sh # 3. Fire
```

### 5.3. **Figure 7(c) & 7(d)**: Compression & Decompression

**WARNING: Takes long time**

Run below on **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./scripts/bench-snappy.sh # 3. Fire
```

### 5.5. **Figure 8**: Dataframe

**WARNING: Takes long time**

Run below on **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./scripts/bench-dataframe.sh # 3. Fire
```

### 5.6. **Figure 9**: GAPBS

**WARNING: Takes long time**

Run below on **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./scripts/bench-gapbs.sh # 3. Fire
```

### 5.7. **Figure 10**: Redis

**WARNING: Takes long time**

Run below on **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./scripts/bench-redis.sh # 3. Fire
```

### 5.7. **Figure 12**: Redis (Bandwidth)

**WARNING: Takes long time**

Run below on **compute node**.

```bash
su - # 1. Login to root. All experiement are conduct in root
cd /root/dilos # 2. Change directory
./scripts/bench-redis-sg.sh # 3. Fire
```
