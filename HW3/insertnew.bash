#!/bin/bash
BEFORE="INSERTMOD SCRIPT:" 
echo "$BEFORE MAKE CLEAN "
make clean
echo "$BEFORE MAKE_ING all "
make
echo "$BEFORE remove the old module "
sudo rmmod usersync
echo "$BEFORE insert the new module "
sudo insmod usersync.ko
