#!/bin/bash

rm result_time.dat
rm result_size.dat
for i in $(seq 1 10); do
    fram_count=$((i*100))
    echo $fram_count >> result_time.dat
    echo $fram_count >> result_size.dat
    sh run.sh -n $fram_count | tail -n1 >> result_time.dat
    ls -l *.img >> result_size.dat
    sh run.sh -n $fram_count -r | tail -n1 >> result_time.dat
    echo "" >> result_time.dat
    echo "" >> result_size.dat
done