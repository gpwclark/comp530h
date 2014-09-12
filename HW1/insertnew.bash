#!/bin/bash
make
sudo rmmod getpinfo
sudo insmod getpinfo.ko
./caller
