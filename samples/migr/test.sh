#!/bin/bash

while true; do
    out/migr
    if [ $? -eq 0 ]; then
        break
    fi
done
