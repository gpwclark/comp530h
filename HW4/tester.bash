#!/bin/bash
echo Make sure everything is newly compiled and inserted
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
sudo ./call_create sched_uwrr 50 &
sudo ./call_create sched_uwrr 100 &
sudo ./call_create sched_uwrr 3 &
sudo ./call_create sched_uwrr 5 &
sudo ./call_create sched_uwrr 8 &
sudo ./call_create sched_uwrr 2 &
sudo ./call_create sched_uwrr 6 &
sudo ./call_create sched_uwrr 7 &
sudo ./call_create sched_uwrr 9 &
sudo ./call_create sched_uwrr 4 &
sudo ./call_create sched_uwrr 10 &
sudo ./call_create sched_uwrr 1 &
sleep 1
sleep 5
echo Prepare to release waiters
echo
../HW3/call_create event_signal 0 
sleep 1
sudo rmmod urrsched
