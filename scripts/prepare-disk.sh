#!/bin/bash
source config.sh

echo "/all.csv: $TRIP_DATA_CSV" >$TRIP_DATA_CSV.manifest
./dilos/scripts/gen-rofs-img.py -o $TRIP_DATA_CSV.raw -m $TRIP_DATA_CSV.manifest

echo "/enwik9.uncompressed: $ENWIKI_UNCOMP" >$ENWIKI_UNCOMP.manifest
./dilos/scripts/gen-rofs-img.py -o $ENWIKI_UNCOMP.raw -m $ENWIKI_UNCOMP.manifest

echo "/enwik9.compressed: $ENWIKI_COMP" >$ENWIKI_COMP.manifest
./dilos/scripts/gen-rofs-img.py -o $ENWIKI_COMP.raw -m $ENWIKI_COMP.manifest

echo "/twitter.sg: $TWITTER" >$TWITTER.manifest
./dilos/scripts/gen-rofs-img.py -o $TWITTER.raw -m $TWITTER.manifest

echo "/twitterU.sg: $TWITTERU" >$TWITTERU.manifest
./dilos/scripts/gen-rofs-img.py -o $TWITTERU.raw -m $TWITTERU.manifest
