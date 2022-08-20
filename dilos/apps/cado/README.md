# Cado - a code-generation language based on Perl.
This is a port of Cado for OSv.

Cado is a simple, general-purpose code-generation language.  The Cado interpreter is ideal for generating source code from rich templates, or creating tool-chains for executing and documenting complex, multi-step transformations.

The distribution contains a rich set of templates and libraries for generating Java, xml, html, [Maven](http://maven.apache.org) projects, refactoring shell scripts, and more.

There is also a new set of operators for interacting with VMware ESXi hypervisors or vCenter servers, based on the venerable [VMware Perl SDK](https://developercenter.vmware.com/web/sdk/55/vsphere-perl).

## Prerequisites
As of this writing, you must build the perl-base image before you can create the Cado image.  (Once Perl has been accepted in the public [Capstan](https://github.com/cloudius-systems/capstan) repository, this will no longer be necessary.)

See osv-apps/perl for more information.

## Porting Changes
There were only a couple of small changes that were necessary:

- eliminated a `system()` call in sp.pl, which was only used to set the local timezone.  Now uses the perl `POSIX` package instead.
- eliminated the shell start-up line from the cado command script.

Both edits are done from the `Makefile` at present.

##How to Build
I use [Capstan](https://github.com/cloudius-systems/capstan) to build.  Since the base is pure perl, you can build the cado base image on any system that supports QEMU.  For example, to build on Mac OS X, you can use [Macports](https://www.macports.org) to install QEMU, install Capstan, and then tell capstan where to find the QEMU binary:

    $ sudo port install qemu
    $ export CAPSTAN_QEMU_PATH=/opt/local/bin/qemu-system-x86_64
    $ capstan build ...

There is a small `build.sh` script to illustrate the top-level build commands for different hypervisors.

##How to Run
You can use Capstan to run.  Here is how it looks running under VMware Fusion 7.1.1:

    $ capstan run -p vmw -i cado-base cadobase00
    Created instance: cadobase00
    OSv v0.22
    eth0: 192.168.98.172
    cado: Version 1.107, 18-Nov-2014.

For the base image, cado just prints its version and exits.

## The Cado Open Source Project
The cado project is hosted on github and sourceforge:

* https://github.com/russt/cado
* http://sourceforge.net/projects/cado

Distribution tarballs can be downloaded from the sourceforge project.
