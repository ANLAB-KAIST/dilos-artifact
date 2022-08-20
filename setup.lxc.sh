#!/bin/bash
cd "$(dirname $0)" || exit
source ./config.sh

lxc-create -n ${LXC_NAME} -t download -- --dist ubuntu --release bionic --arch amd64 --no-validate 


echo "lxc.net.0.ipv4.address = 10.0.3.3/24" >> "/var/lib/lxc/${LXC_NAME}/config"
echo "lxc.net.0.ipv4.gateway = 10.0.3.1" >> "/var/lib/lxc/${LXC_NAME}/config"
echo "lxc.apparmor.allow_incomplete = 1" >> "/var/lib/lxc/${LXC_NAME}/config"

sleep 5

lxc-start -n ${LXC_NAME} -d

sleep 5

lxc-attach -n ${LXC_NAME} -- bash -c "systemctl stop systemd-resolved"
lxc-attach -n ${LXC_NAME} -- bash -c "systemctl disable systemd-resolved"
lxc-attach -n ${LXC_NAME} -- bash -c "rm /etc/resolv.conf"
lxc-attach -n ${LXC_NAME} -- bash -c "echo 'nameserver 8.8.8.8' > /etc/resolv.conf"


lxc-stop -n ${LXC_NAME}