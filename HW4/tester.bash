#!/bin/bash
./insertnew.bash
cd ../HW3
./insertnew.bash
cd ../HW4
echo
echo
echo Prepart to create event and start data collecting:
sleep 5
call_create_HW3 event_create HW4 #create the event to wait on, the callers will make a call to wait on event 0
sleep 2
sudo ./call_create sched_uwrr 1 &
sleep 1
sudo ./call_create sched_uwrr 10 &
sleep 1
sudo ./call_create sched_uwrr 100 &
sleep 1
sudo ./call_create sched_uwrr 1000 &
sleep 5
echo Prepare to release waiters
echo
../HW3/call_create event_signal 0 
