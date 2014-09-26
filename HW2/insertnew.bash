#!/bin/bash
make clean
make
sudo rmmod getpinfo
sudo insmod getpinfo.ko
./caller
