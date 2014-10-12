#!/bin/bash
./call_create event_create elephant
./call_create event_create bobby
./call_create event_create fred

./call_create event_id elephant
./call_create event_id bobby
./call_create event_id fred

sleep 5
./call_create event_wait 2 0 &
sleep 2
./call_create event_wait 2 0 &
sleep 2
./call_create event_wait 2 0 &

sleep 2
./call_create event_signal 2
