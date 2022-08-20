#!/bin/bash
source config.sh

cd dilos
gdb -iex "set auto-load safe-path ." -ex connect -ex "osv syms" build/release/loader.elf
