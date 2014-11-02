#!/bin/bash
echo Rotate the logs:
sudo logrotate --force /etc/logrotate.conf
sudo su -c "cat /dev/null > /var/log/kernel.log"
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
sudo ./call_create sched_uwrr 1 &
sleep 1
sudo ./call_create sched_uwrr 2 &
sleep 1
sudo ./call_create sched_uwrr 3 &
sleep 1
sudo ./call_create sched_uwrr 4 &
sleep 1
sudo ./call_create sched_uwrr 5 &
sleep 1
sudo ./call_create sched_uwrr 6 &
sleep 1
sudo ./call_create sched_uwrr 7 &
sleep 1
sudo ./call_create sched_uwrr 8 &
sleep 1
sudo ./call_create sched_uwrr 9 &
sleep 1
sudo ./call_create sched_uwrr 10 &
sleep 5
echo Prepare to release waiters
echo
../HW3/call_create event_signal 0 
sudo cp /var/log/kernel.log .
sudo chown $USER kernel.log
sudo chmod 666 kernel.log
git commit -m 'updated log from testing' * ; git push origin master
