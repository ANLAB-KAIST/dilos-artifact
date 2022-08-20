#!/bin/bash
cd "$(dirname $0)" || exit

pushd aifm/shenango || exit
./scripts/setup_machine.sh
popd || exit
