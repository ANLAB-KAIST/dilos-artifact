#!/bin/sh
#
#build.sh - build images for various hypervisor targets.
#

imagename=perlhello
hypervisors="qemu vbox vmw gce"
hypervisors="vmw"

for target in $hypervisors 
do
    echo capstan build -p $target -v $imagename
    capstan build -p $target -v $imagename
done
