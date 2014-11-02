#!/bin/bash
./call_create event_create myev
echo
./call_create event_create myev2
echo
./call_create event_create myev3
echo
sleep 5
./call_create event_id myev3
echo
./call_create event_id myev2
echo
./call_create event_id myev
sleep 1
echo -----
./call_create event_wait 0 0 &
./call_create event_wait 0 0 &
./call_create event_wait 0 0 &
./call_create event_wait 0 0 &
./call_create event_wait 0 0 &
sleep 2
./call_create event_wait 0 1 &
./call_create event_wait 0 1 &
./call_create event_wait 0 1 &
./call_create event_wait 0 1 &
echo
sleep 5
echo -----

#./call_create event_signal 0
#echo
#sleep 5
#echo -----
#./call_create event_signal 0
#echo
#sleep 5
#echo -----
#./call_create event_signal 0
#echo
#sleep 5
#echo -----
#./call_create event_signal 0
#echo
#sleep 5
#echo -----
#./call_create event_signal 0
#echo
#sleep 5
#echo -----
./call_create event_destroy 0
echo
sleep 5
echo -----
echo END OF TEST
