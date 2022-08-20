#!/bin/sh

ifconfig $1 down
brctl delif $OSV_BRIDGE $1
