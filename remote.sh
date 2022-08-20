#!/bin/bash
source config.sh

today=$(date +%F_%H-%d-%S)

run-remote() {
    ssh $MS_IP "$@"
}

update() {
    run-remote "cd $MS_PATH; git pull"
}

build() {
    run-remote "mkdir -p $MS_PATH/build/remote; cd $MS_PATH/build/remote; cmake -G Ninja \"$MS_PATH/remote\"; ninja remote_server"
}

clean() {
    run-remote "rm -rf $MS_PATH/build/remote"
}

setup() {
    run-remote "cd $MS_PATH; ./setup-remote.sh"
}

up() {
    run-remote "cd $MS_PATH/build/remote/server; nohup numactl -N $MS_NODE -m $MS_NODE ./remote_server $MS_PORT $MS_MEMORY_GB > $today.log 2>&1 &"
}

down() {
    run-remote "pkill -f ./remote_server"
}

case $1 in
update | build | setup | up | down)
    $1
    ;;
*)
    echo "unknown: $1"
    ;;
esac
