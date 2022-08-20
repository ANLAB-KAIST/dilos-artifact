#!/bin/bash
JAVA_VERSION=$1

DOWNLOAD_URL_SUFFIX=`wget -c -qO- https://cdn.azul.com/zulu/bin/ | grep -o '<a href=['"'"'"][^"'"'"']*['"'"'"]' | grep "jdk${JAVA_VERSION}.*linux.*64.tar.gz\"" | grep -o "zulu.*tar.gz" | tail -n 1`
DOWNLOAD_URL="https://cdn.azul.com/zulu/bin/$DOWNLOAD_URL_SUFFIX"
VERSION=`echo $DOWNLOAD_URL | grep -o "zulu${JAVA_VERSION}.*linux_x64"`

if [ "$2" = "version" ]; then
echo $VERSION
else
echo $DOWNLOAD_URL
fi
