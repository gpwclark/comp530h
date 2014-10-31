#!/bin/bash
./insertnew.bash
../HW3/insertnew.bash
../HW3/call_create event_create HW4 #create the event to wait on, the callers will make a call to wait on event 0
sudo ./call_create sched_uwrr 10 &
sudo ./call_create sched_uwrr 10 &
sudo ./call_create sched_uwrr 10 &
../HW3/call_create event_signal 0 
