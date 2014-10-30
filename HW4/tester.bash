#!/bin/bash
sudo ./call_create sched_uwrr 1
while [  1 ]; do
    for i in $( ps ); do
        echo i
    done
    sleep 3
done
