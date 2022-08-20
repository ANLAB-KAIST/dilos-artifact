#!/bin/bash
cd "$(dirname $0)" || exit 0
source ./config.sh


echo "Building AIFM"

./build.aifm.sh

echo "Setup AIFM"
./setup.aifm.sh


echo "Run Dataframe"
pushd aifm/aifm/exp/fig7/aifm_no_offload || exit 0
#./run.sh
popd || exit


echo "Run Snappy"
pushd aifm/aifm/exp/fig11a/aifm || exit 0
./run.sh
popd || exit 0

pushd aifm/aifm/exp/fig11b/aifm || exit 0
./run.sh
popd || exit 0
