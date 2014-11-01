#!/bin/bash
./insertnew.bash
cd ../HW3
./insertnew.bash
cd ../HW4
../HW3/call_create event_create HW4 #create the event to wait on, the callers will make a call to wait on event 0
sleep 5
sudo ./call_create sched_uwrr 10 &
sleep 1
sudo ./call_create sched_uwrr 10 &
sleep 1
sudo ./call_create sched_uwrr 10 &
sleep 1
../HW3/call_create event_signal 0 
