#!/bin/bash
echo "Check if extfat is in C"

if [[ "src/extfat.c" == *.c ]]; then
    echo "Code is written in C"
    result=0
else   
    result=1
fi

exit $result