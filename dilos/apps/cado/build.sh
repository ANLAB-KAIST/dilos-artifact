#!/bin/sh
#
#build.sh - build images for various hypervisor targets.
#

verbose=-v
verbose=

imagename=cado-base
hypervisors="qemu vbox vmw gce"
hypervisors="vmw"

for target in $hypervisors 
do
    echo capstan build -p $target $verbose $imagename
    capstan build -p $target $verbose $imagename
done
