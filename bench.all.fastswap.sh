#!/bin/bash
cd "$(dirname $0)" || exit 0

echo "Seq"

./bench.seq.fastswap.sh

echo "QuickSort"

./bench.quicksort.fastswap.sh

echo "KMeans"

./bench.kmeans.fastswap.sh


echo "Snappy"

./bench.snappy.fastswap.sh


echo "GAPBS"

./bench.gapbs.fastswap.sh

echo "DataFrame"


./bench.dataframe.fastswap.sh

echo "Redis"

./bench.redis.fastswap.sh