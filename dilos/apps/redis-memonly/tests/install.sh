#!/bin/sh
set -e

tests_dir=${0%/*}

cd $tests_dir/..

VERSION=3.0.0-beta3
rm -Rf upstream
mkdir upstream
cd upstream
wget https://github.com/antirez/redis/archive/$VERSION.tar.gz
tar zxvf $VERSION.tar.gz
mv redis-$VERSION redis
cd redis
make
cd ../..

echo export REDIS_BENCHMARK="`pwd`/upstream/redis/src/redis-benchmark" > tests/setenv.sh
