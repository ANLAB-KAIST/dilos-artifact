#!/bin/bash
cd "$(dirname $0)" || exit
source ./config.sh

DOWNLOAD_PATH="${LXC_ROOTFS}/mnt"
mkdir -p "${DOWNLOAD_PATH}"
pushd "${DOWNLOAD_PATH}"
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/enwik9.compressed
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/enwik9.uncompressed

wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2015-01.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2015-02.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2015-03.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2016-01.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2016-02.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2016-03.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2016-04.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2016-05.csv
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/yellow_tripdata_2016-06.csv

head=yellow_tripdata_2016-01.csv

cat $head >all.csv

for file in $(ls *.csv); do
    if [ "$file" = "all.csv" ]; then
        continue
    fi
    if [ "$file" = "$head" ]; then
        continue
    fi
    awk '{if (NR > 1) print $0}' $file >>all.csv
done


wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitter.sg.00.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitter.sg.01.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitter.sg.02.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitter.sg.03.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitter.sg.04.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitter.sg.05.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitter.sg.06.gz

gunzip -c twitter.sg.00 twitter.sg.01 twitter.sg.02 twitter.sg.03 twitter.sg.04 twitter.sg.05 twitter.sg.06 >twitter.sg

wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitterU.sg.00.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitterU.sg.01.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitterU.sg.02.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitterU.sg.03.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitterU.sg.04.gz
wget https://github.com/ANLAB-KAIST/dilos-artifact/releases/download/dataset/twitterU.sg.05.gz

gunzip -c twitterU.sg.00 twitterU.sg.01 twitterU.sg.02 twitterU.sg.03 twitterU.sg.04 twitterU.sg.05 >twitterU.sg

