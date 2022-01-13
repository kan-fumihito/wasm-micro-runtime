#!/bin/bash


for i in $(seq 1 10); do
    frame_count=$((i*100))
    rm result_time_dump_$frame_count.dat
    rm result_time_restore_$frame_count.dat
    rm result_size.dat
    echo $frame_count >> result_size.dat
    for j in $(seq 1 50); do
        echo $j
        sh run.sh -n $frame_count | tail -n1 >> result_time_dump_$frame_count.dat
        sh run.sh -n $frame_count -r | tail -n1 >> result_time_restore_$frame_count.dat
    done
    ls -l *.img >> result_size.dat
    echo "" >> result_size.dat
done