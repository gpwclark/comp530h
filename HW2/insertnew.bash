#!/bin/bash
BEFORE="INSERTMOD SCRIPT:" 
echo "$BEFORE MAKE CLEAN "
make clean
echo "$BEFORE MAKE_ING all "
make
echo "$BEFORE remove the old module "
sudo rmmod getpinfo
echo "$BEFORE insert the new module "
sudo insmod getpinfo.ko
echo "$BEFORE execute the dummy programs"
sleep 1
./dummy & ./dummy & ./dummy &
