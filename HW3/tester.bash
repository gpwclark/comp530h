#!/bin/bash
./call_create 0
echo
./call_create 1
echo
sleep 5
./call_create 2
echo
sleep 5
echo -----
./call_create 3 &
./call_create 3 &
./call_create 3 &
./call_create 3 &
./call_create 4 &
./call_create 4 &
./call_create 4 &
./call_create 4 &
echo
sleep 5
echo -----

./call_create 5 &
echo
sleep 5
echo -----
./call_create 5 &
echo
sleep 5
echo -----
./call_create 6 &
echo
sleep 5
echo -----
echo END OF TEST
