#!/bin/bash
make clean
make
sudo rmmod vmlogger
sudo insmod vmlogger.ko
