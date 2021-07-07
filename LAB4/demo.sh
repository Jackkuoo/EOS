#!/bin/sh

set -x
# set -e

rmmod -f mydev
insmod mydev.ko

./writer jackkuo &
./reader 192.168.0.30 8765 /dev/mydev
