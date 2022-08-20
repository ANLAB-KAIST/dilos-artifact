#!/bin/bash

package=$1

arch=$2

letter=${package:0:1}

wget -t 1 -qO- http://mirrors.kernel.org/fedora/development/rawhide/Everything/x86_64/os/Packages/$letter/ | grep "$package-[0-9].*$arch" | sed -e "s/<a href\=\"$package-\(.*\)\.$arch\.rpm\".*/\1/g" | tail -1
