***OSv was originally designed and implemented by Cloudius Systems (now ScyllaDB) however
 currently it is being maintained and enhanced by a small community of volunteers.
 If you are into systems programming or want to learn and help us improve OSv, then please
 contact us on [OSv Google Group forum](https://groups.google.com/forum/#!forum/osv-dev).
 For details on how to format and send patches, please read
 [this wiki](https://github.com/cloudius-systems/osv/wiki/Formatting-and-sending-patches)
 (__we do NOT accept pull requests__).***

# OSv

OSv is an open-source versatile modular **unikernel** designed to run single **unmodified
Linux application** securely as microVM on top of a hypervisor, when compared to traditional
operating systems which were designed for a vast range of physical machines. Built from
the ground up for effortless deployment and management of microservices
and serverless apps, with superior performance.

OSv has been designed to run unmodified x86-64 Linux
binaries **as is**, which effectively makes it a **Linux binary compatible unikernel**
(for more details about Linux ABI compatibility please read
[this doc](https://github.com/cloudius-systems/osv/wiki/OSv-Linux-ABI-Compatibility)).
In particular OSv can run many managed language runtimes including
[**JVM**](https://github.com/cloudius-systems/osv-apps/tree/master/java-example),
**Python** [**2**](https://github.com/cloudius-systems/osv-apps/tree/master/python2x) and
[**3**](https://github.com/cloudius-systems/osv-apps/tree/master/python3x),
[**Node.JS**](https://github.com/cloudius-systems/osv-apps/tree/master/node-from-host),
[**Ruby**](https://github.com/cloudius-systems/osv-apps/tree/master/ruby-example), **Erlang**,
and applications built on top of those runtimes.
It can also run applications written in languages compiling directly to native machine code like
**C**, **C++**,
[**Golang**](https://github.com/cloudius-systems/osv-apps/tree/master/golang-httpserver)
and [**Rust**](https://github.com/cloudius-systems/osv-apps/tree/master/rust-httpserver)
as well as native images produced
by [**GraalVM**](https://github.com/cloudius-systems/osv-apps/tree/master/graalvm-example)
and [WebAssembly/Wasmer](https://github.com/cloudius-systems/osv-apps/tree/master/webassembly).

OSv can boot as fast as **~5 ms** on Firecracker using as low as 15 MB of memory.
OSv can run on many hypervisors including QEMU/KVM,
[Firecracker](https://github.com/cloudius-systems/osv/wiki/Running-OSv-on-Firecracker),
Xen, [VMWare](https://github.com/cloudius-systems/osv/wiki/Running-OSv-on-VMware-ESXi),
[VirtualBox](https://github.com/cloudius-systems/osv/wiki/Running-OSv-on-VirtualBox) and
Hyperkit as well as open clouds like AWS EC2, GCE and OpenStack.

For more information about OSv, see the [main wiki page](https://github.com/cloudius-systems/osv/wiki)
and http://osv.io/.

## Building and Running Apps on OSv

In order to run an application on OSv, one needs to build an image by fusing OSv kernel, and
the application files together. This, in high level can be achieved in two ways, either:
- by using the shell script located at `./scripts/build`
 that builds the kernel from sources and fuses it with application files, or
- by using the [capstan tool](https://github.com/cloudius-systems/capstan) that uses *pre-built
 kernel* and combines it with application files to produce a final image.

If your intention is to try to run your app on OSv with the least effort possible, you should pursue the *capstan*
route. For introduction please read this 
[crash course](https://github.com/cloudius-systems/osv/wiki/Build-and-run-apps-on-OSv-using-Capstan).
For more details about *capstan* please read 
this more detailed [documentation](https://github.com/cloudius-systems/capstan#documentation). Pre-built OSv kernel files
(`ovs-loader.qemu`) can be automatically downloaded by *capstan* from 
the [OSv regular releases page](https://github.com/cloudius-systems/osv/releases) or manually from 
the [nightly releases repo](https://github.com/osvunikernel/osv-nightly-releases/releases/tag/ci-master-latest).

If you are comfortable with make and GCC toolchain and want to try the latest OSv code, then you should
read this [part of the readme](#setting-up-development-environment) to guide you how to set up your
 development environment and build OSv kernel and application images.

## Releases

We aim to release OSv 2-3 times a year. You can find the [latest one on github](https://github.com/cloudius-systems/osv/releases)
along with number of published artifacts including kernel and some modules.

In addition, we have set up [Travis-based CI/CD pipeline](https://travis-ci.org/github/cloudius-systems/osv) where each
commit to the master and ipv6 branches triggers full build of the latest kernel and publishes some artifacts to 
the [nightly releases repo](https://github.com/osvunikernel/osv-nightly-releases/releases). Each commit also
triggers publishing of new Docker "build tool chain" images to the [Docker hub](https://hub.docker.com/u/osvunikernel).

## Design

Good bit of the design of OSv is pretty well explained in 
the [Components of OSv](https://github.com/cloudius-systems/osv/wiki/Components-of-OSv) wiki page. You 
can find even more information in the original 
[USENIX paper and its presentation](https://www.usenix.org/conference/atc14/technical-sessions/presentation/kivity).

In addition, you can find a lot of good information about design of specific OSv components on
the [main wiki page](https://github.com/cloudius-systems/osv/wiki) and http://osv.io/ and http://blog.osv.io/.
Unfortunately, some of that information may be outdated (especially on http://osv.io/), so it is always
best to ask on the [mailing list](https://groups.google.com/forum/#!forum/osv-dev) if in doubt.

## Metrics and Performance

There are no official **up-to date** performance metrics comparing OSv to other unikernels or Linux.
In general OSv lags behind Linux in disk-I/O-intensive workloads partially due to coarse-grained locking 
in VFS around read/write operations as described in this [issue](https://github.com/cloudius-systems/osv/issues/450).
In network-I/O-intensive workloads, OSv should fare better (or at least used to as Linux has advanced a lot since)
as shown with performance tests of Redis and [Memcached](https://github.com/cloudius-systems/osv/wiki/OSv-Case-Study:-Memcached).
You can find some old "numbers" on the main wiki, http://osv.io/benchmarks and some papers listed at the bottom of this readme.

So OSv is probably not best suited to run MySQL or ElasticSearch, but should deliver pretty solid performance for general
 stateless applications like microservices or serverless (at least as some papers show).

### Kernel Size

At this moment (as of May 2020) the size of the uncompressed OSv kernel (`kernel.elf` artifact) is around
6.7 MB (the compressed is ~ 2.7 MB). This is not that small comparing to Linux kernel and quite large comparing
to other unikernels. However, bear in mind that OSv kernel (being unikernel) provides **subset** of functionality
 of the following Linux libraries (see their approximate size on Linux host):
- `libresolv.so.2` (_100 K_)
- `libc.so.6` (_2 MB_)
- `libm.so.6` (_1.4 MB_)
- `ld-linux-x86-64.so.2` (_184 K_)
- `libpthread.so.0` (_156 K_)
- `libdl.so.2` (_20 K_)
- `librt.so.1` (_40 K_)
- `libstdc++.so.6` (_2 MB_)
- `libaio.so.1` (_16 K_)
- `libxenstore.so.3.0` (_32 K_)
- `libcrypt.so.1` (_44 K_)

The equivalent static version of `libstdc++.so.6` is actually linked `--whole-archive` so that
any C++ apps can run without having to add `libstdc++.so.6` to the image (whether it needs it or not).
Finally, OSv kernel comes with ZFS implementation which in theory later can be extracted as a 
[separate library](https://github.com/cloudius-systems/osv/issues/1009). The
point of this is to illustrate that comparing OSv kernel size to Linux kernel size does not
quite make sense.

### Boot Time

OSv with _Read-Only FS with networking off_ can boot as fast as **~5 ms** on Firecracker 
and even faster around **~3 ms** on QEMU with the microvm machine. However, in general the boot time
will depend on many factors like hypervisor including settings of individual para-virtual devices, 
filesystem (ZFS, ROFS or RAMFS) and some boot parameters. Please note that by default OSv images
get built with ZFS filesystem.

For example, the boot time of ZFS image on Firecracker is around ~40 ms and regular QEMU around 200 ms these days. Also,
newer versions of QEMU (>=4.0) are typically faster to boot. Booting on QEMU in PVH/HVM mode (aka direct kernel boot, enabled 
by `-k` option of `run.py`) should always be faster as OSv is directly invoked in 64-bit long mode. Please see
[this Wiki](https://github.com/cloudius-systems/osv/wiki/OSv-boot-methods-overview) for the brief review of the boot
methods OSv supports.

Finally, some boot parameters passed to the kernel may affect the boot time:
- `--console serial` - this disables VGA console that is [slow to initialize](https://github.com/cloudius-systems/osv/issues/987) and can shave off 60-70 ms on QEMU
- `--nopci` - this disables enumeration of PCI devices especially if we know none are present (QEMU with microvm or Firecracker) and can shave off 10-20 ms 
- `--redirect=/tmp/out` - writing to the console can impact the performance quite severely (30-40%) if application logs 
a lot, so redirecting standard output and error to a file might speed up performance quite a lot

You can always see boot time breakdown by adding `--bootchart` parameter:
```
./scripts/run.py -e '--bootchart /hello'
OSv v0.54.0-197-g1f0df4e4
eth0: 192.168.122.15
	disk read (real mode): 25.85ms, (+25.85ms)
	uncompress lzloader.elf: 45.11ms, (+19.26ms)
	TLS initialization: 45.72ms, (+0.61ms)
	.init functions: 47.61ms, (+1.89ms)
	SMP launched: 48.08ms, (+0.47ms)
	VFS initialized: 50.99ms, (+2.91ms)
	Network initialized: 51.12ms, (+0.14ms)
	pvpanic done: 51.25ms, (+0.13ms)
	pci enumerated: 61.55ms, (+10.29ms)
	drivers probe: 61.55ms, (+0.00ms)
	drivers loaded: 135.91ms, (+74.36ms)
	ROFS mounted: 136.98ms, (+1.07ms)
	Total time: 138.16ms, (+1.18ms)
Cmdline: /hello
Hello from C code
```

### Memory Utilization

OSv needs at least 15 M of memory to run a _hello world_ app. Even though it is half of
what it was 2 years ago, it is still quite a lot comparing to other unikernels. We are planning to further lower
this number by reducing size of the kernel, adding [self-tuning logic to L1/L2 memory pools](https://github.com/cloudius-systems/osv/issues/1013) and
making application threads use [lazily allocated stacks](https://github.com/cloudius-systems/osv/issues/143).

## Testing

OSv comes with around 130 unit tests that get executed upon every commit and run on ScyllaDB servers. There are also number of extra
tests located under `tests/` sub-tree that are not automated at this point.

You can run unit tests in number of ways:
```
./scripts/build check                  # Create ZFS test image and run all tests on QEMU

./scripts/build check fs=rofs          # Create ROFS test image and run all tests on QEMU

./scripts/build image=tests && \       # Create ZFS test image and run all tests on Firecracker
./scripts/test.py -p firecracker

./scripts/build image=tests && \       # Create ZFS test image and run all tests on QEMU
./scripts/test.py -p qemu_microvm      # with microvm machine
```

In addition, there is an [Automated Testing Framework](https://github.com/cloudius-systems/osv/wiki/Automated-Testing-Framework)
that can be used to run around 30 real apps, some of them
under stress using `ab` or `wrk` tools. The intention is to catch any regressions that might be missed
by unit tests.

Finally, one can use [Docker files](https://github.com/cloudius-systems/osv/tree/master/docker#docker-osv-builder) to
test OSv on different Linux distribution.

## Setting up Development Environment

OSv can only be built on a 64-bit x86 Linux distribution. Please note that
this means the "x86_64" or "amd64" version, not the 32-bit "i386" version.

In order to build OSv kernel you need a physical or virtual machine with Linux distribution on it and GCC toolchain and
all necessary packages and libraries OSv build process depends on. The fastest way to set it up is to use the
[Docker files](https://github.com/cloudius-systems/osv/tree/master/docker#docker-osv-builder) that OSv comes with.
You can use them to build your own Docker image and then start it in order to build OSv kernel or run an app on OSv inside of it.
Please note that the main docker file depends on pre-built base **Docker images** for 
[Ubuntu](https://hub.docker.com/repository/docker/osvunikernel/osv-ubuntu-19.10-builder-base) 
or [Fedora](https://hub.docker.com/repository/docker/osvunikernel/osv-fedora-31-builder-base) 
that get published to DockerHub upon every commit. This should speed up building the final images
as all necessary packages should already be part of the base images.

Alternatively, you can manually clone OSv repo and use [setup.py](https://github.com/cloudius-systems/osv/blob/master/scripts/setup.py)
to install all required packages and libraries, as long as it supports your Linux distribution, and you have both git 
and python 3 installed on your machine:
```bash
git clone https://github.com/cloudius-systems/osv.git
cd osv && git submodule update --init --recursive
./scripts/setup.py
```

The `setup.py` recognizes and installs packages for number of Linux distributions including Fedora, Ubuntu, 
[Debian](https://github.com/cloudius-systems/osv/wiki/Building-OSv-on-Debian-stable), LinuxMint and RedHat ones 
(Scientific Linux, NauLinux, CentOS Linux, Red Hat Enterprise Linux, Oracle Linux). Please note that we actively
maintain and test only Ubuntu and Fedora, so your mileage with other distributions may vary. The `setup.py`
is actually used by Docker files internally to achieve the same result. 

### IDEs

If you like working in IDEs, we recommend either [Eclipse CDT](https://www.eclipse.org/cdt/) which can be setup
as described in this [wiki page](https://github.com/cloudius-systems/osv/wiki/Working-With-Eclipse-CDT) or 
[CLion from JetBrains](https://www.jetbrains.com/clion/) which can be setup to work with OSv makefile using
so called compilation DB as described in this [guide](https://www.jetbrains.com/help/clion/managing-makefile-projects.html).

## Building OSv Kernel and Creating Images

Building OSv is as easy as using the shell script `./scripts/build`
that orchestrates the build process by delegating to the main [makefile](https://github.com/cloudius-systems/osv/blob/master/Makefile)
to build the kernel and by using number of Python scripts like `./scripts/module.py` 
to build application and *fuse* it together with the kernel
into a final image placed at `./build/release/usr.img` (or `./build/$(arch)/usr.img` in general).
Please note that *building an application* does not necessarily mean building from sources as in many 
cases the application binaries would be located on and copied from the Linux build machine
using the shell script `./scripts/manifest_from_host.sh`
(see [this Wiki page](https://github.com/cloudius-systems/osv/wiki/Running-unmodified-Linux-executables-on-OSv) for details).

The shell script `build` can be used as the examples below illustrate:
```bash
# Create default image that comes with command line and REST API server
./scripts/build

# Create image with native-example app
./scripts/build -j4 fs=rofs image=native-example

# Create image with spring boot app with Java 10 JRE
./scripts/build JAVA_VERSION=10 image=openjdk-zulu-9-and-above,spring-boot-example

 # Create image with 'ls' executable taken from the host
./scripts/manifest_from_host.sh -w ls && ./scripts/build --append-manifest

# Create test image and run all tests in it
./scripts/build check

# Clean the build tree
./scripts/build clean
```

Command nproc will calculate the number of jobs/threads for make and `./scripts/build` automatically.
Alternatively, the environment variable MAKEFLAGS can be exported as follows:

```
export MAKEFLAGS=-j$(nproc)
```

In that case, make and scripts/build do not need the parameter -j.

For details on how to use the build script, please run `./scripts/build --help`.

The `./scripts/build` creates the image `build/last/usr.img` in qcow2 format.
To convert this image to other formats, use the `./scripts/convert`
tool, which can convert an image to the vmdk, vdi or raw formats.
For example:

```
./scripts/convert raw
```

By default, OSv kernel gets built for x86_64 architecture, but it is also possible
 to build one for ARM by adding **arch** parameter like so:
```bash
./scripts/build arch=aarch64
```
At this point cross-compiling the **aarch64** version of OSv is only supported
on Fedora and relevant aarch64 gcc and libraries' binaries can be downloaded using
the `./scripts/download_fedora_aarch64_packages.py` script.
Please note that simple "hello world" app should work just fine, but overall the ARM part of OSv has not been
 as well maintained and tested as x86_64 due to the lack of volunteers. In addition,
 the same simple example can successfully run on QEMU on Raspberry PI 4 with KVM acceleration enabled.
 For more information about the aarch64 port please read [this Wiki page](https://github.com/cloudius-systems/osv/wiki/AArch64).

### Filesystems

At the end of the boot process, OSv dynamic linker loads an application ELF and any related libraries
 from the filesystem on a disk that is part of the image. By default, the images built by `./scripts/build`
 contain a disk formatted as ZFS, which you can read more about [here](https://github.com/cloudius-systems/osv/wiki/ZFS).
 ZFS is a great read-write file system and may be a perfect fit if you want to run MySQL on OSv. However, it may be an overkill
 if you want to run stateless apps in which case you may consider 
 [Read-Only FS](https://github.com/cloudius-systems/osv/commit/cd449667b7f86721095ddf4f9f3f8b87c1c414c9). Finally,
 you can also have OSv read the application binary from RAMFS, in which case the filesystem get embedded as part of
 the kernel ELF. You can specify which filesystem to build image disk as
  by setting parameter `fs` of `./scripts/build` to one of the three values -`zfs`, `rofs` or `ramfs`.

In addtion, one can mount NFS filesystem, which had been recently transformed to be a shared library pluggable as a [module](https://github.com/cloudius-systems/osv/tree/master/modules/nfs), and newly implemented [Virtio-FS filesystem](https://stefanha.github.io/virtio/virtio-fs.html#x1-41500011). The NFS and Virtio-FS mounts can be setup by adding proper entry `/etc/fstab` or by passing a boot parameter as explained in this [commit comments](https://github.com/cloudius-systems/osv/commit/47c7e9268ff96f67f4649bb6c63685a5c2d74f00).

## Running OSv

Running an OSv image, built by `scripts/build`, is as easy as:
```bash
./scripts/run.py
```

By default, the `run.py` runs OSv under KVM, with 4 vCPUs and 2 GB of memory. 
You can control these and tens of other ones by passing relevant parameters to 
the `run.py`. For details, on how to use the script, please run `./scripts/run.py --help`.

The `run.py` can run OSv image on QEMU/KVM, Xen and VMware. If running under KVM you can terminate by hitting Ctrl+A X.

Alternatively, you can use `./scripts/firecracker.py` to run OSv on [Firecracker](https://firecracker-microvm.github.io/). 
This script automatically downloads firecracker binary if missing, and accepts number of parameters like number ot vCPUs, memory
named exactly like `run.py` does. You can learn more about running OSv on Firecracker 
from this [wiki](https://github.com/cloudius-systems/osv/wiki/Running-OSv-on-Firecracker). 

Please note that in order to run OSv with the best performance on Linux under QEMU or Firecracker you need KVM enabled 
(this is only possible on *physical* Linux machines, EC2 "bare metal" (i3) instances or VMs that support nested virtualization with KVM on). 
The easiest way to verify if KVM is enabled is to check if `/dev/kvm` is present, and your user account can read from and write to it. 
Adding your user to the kvm group may be necessary like so:
```bash
usermod -aG kvm <user name>
```

For more information about building and running JVM, Node.JS, Python and other managed runtimes as well as Rust, Golang or C/C++ apps
 on OSv, please read this [wiki page](https://github.com/cloudius-systems/osv/wiki#running-your-application-on-osv). 
 For more information about various example apps you can build and run on OSv, please read 
 [the osv-apps repo README](https://github.com/cloudius-systems/osv-apps#osv-applications).

### Networking

By default, the `run.py`  starts OSv with
 [user networking/SLIRP](https://wiki.qemu.org/Documentation/Networking#User_Networking_.28SLIRP.29) on. 
To start OSv with more performant external networking, you need to enable `-n` and `-v` options like so:

```
sudo ./scripts/run.py -nv
```

The -v is for KVM's vhost that provides better performance
and its setup requires tap device and thus we use sudo.

By default, OSv spawns a `dhcpd`-like thread that automatically configures virtual NICs.
A static configuration can be done within OSv by configuring networking like so:

```
ifconfig virtio-net0 192.168.122.100 netmask 255.255.255.0 up
route add default gw 192.168.122.1
```

To enable networking on Firecracker, you have to explicitly enable `-n` option
to `firecracker.py`.

Finally, please note that the master branch of OSv only implements IPV4 subset of networking stack.
If you need IPV6, please build from [ipv6 branch](https://github.com/cloudius-systems/osv/tree/ipv6)
 or use IPV6 kernel published to [nightly releases repo](https://github.com/osvunikernel/osv-nightly-releases/releases/tag/ci-ipv6-latest). 

## Debugging, Monitoring, Profiling OSv

- OSv can be debugged with gdb; for more details please read this
 [wiki](https://github.com/cloudius-systems/osv/wiki/Debugging-OSv)
- OSv kernel and application can be traced and profiled; for more details please read 
this [wiki](https://github.com/cloudius-systems/osv/wiki/Trace-analysis-using-trace.py)
- OSv comes with the admin/monitoring REST API server; for more details please read 
[this](https://github.com/cloudius-systems/osv/wiki/Command-Line-Interface-(CLI)) and
 [that wiki page](https://github.com/cloudius-systems/osv/wiki/Using-OSv-REST-API). There is also
 lighter [monitoring REST API module](https://github.com/cloudius-systems/osv/commit/aa32614221254ce300f401bb99c506b528b85682) 
 that is effectively a read-only subset of the former one. 
 
## FAQ and Contact

If you want to learn more about OSv or ask questions, 
please contact us on [OSv Google Group forum](https://groups.google.com/forum/#!forum/osv-dev).
You can also follow us on [Twitter](https://twitter.com/osv_unikernel).

## Papers and Articles about OSv

List of somewhat newer articles about OSv found on the Web:
* [Unikernels vs Containers: An In-Depth Benchmarking Study in the context of Microservice Applications](https://biblio.ugent.be/publication/8582433/file/8582438)
* [Towards a Practical Ecosystem of Specialized OS Kernels](http://cs.iit.edu/~khale/docs/diver-ross19.pdf)
* [A Performance Evaluation of Unikernels](https://pdfs.semanticscholar.org/d956/f72dbc65301578dc95e0f751f4ae7c09d831.pdf)
* [Security Perspective on Unikernels](https://arxiv.org/pdf/1911.06260.pdf)
* [Performance Evaluation of OSv for Server Applications](http://www.cs.utah.edu/~peterm/prelim-osv-performance.pdf)
* [Time provisioning Evaluation of KVM, Docker and Unikernels in a Cloud Platform](https://tiagoferreto.github.io/pubs/2016ccgrid_xavier.pdf)
* [Unikernels - Beyond Containers to the Next Generation of the Cloud](https://theswissbay.ch/pdf/_to_sort/O'Reilly/unikernels.pdf)

You can find some older articles and presentations at http://osv.io/resources and http://blog.osv.io/.
