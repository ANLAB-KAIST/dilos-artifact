#!/bin/bash

#DOWNLOAD_URL_SUFFIX=`wget -c -qO- https://cdn.azul.com/zulu/bin/ | grep -o '<a href=['"'"'"][^"'"'"']*['"'"'"]' | grep "jdk8.*linux.*64.tar.gz\"" | grep -o "zulu.*tar.gz" | tail -n 1`
#DOWNLOAD_URL="https://cdn.azul.com/zulu/bin/$DOWNLOAD_URL_SUFFIX"
#echo $DOWNLOAD_URL
echo "https://cdn.azul.com/zulu/bin/zulu8.40.0.25-ca-jdk8.0.222-linux_x64.tar.gz"
