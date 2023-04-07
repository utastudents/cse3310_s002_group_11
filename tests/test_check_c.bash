#!/bin/bash
echo "Test to if code is in C"

count=0

if [[ "src/extfat.c" == *.c ]]; then
    ((count++))
fi

if [[ "src/cmdLine.c" == *.c ]]; then
    ((count++))
fi

if [[ "src/mmap.c" == *.c ]]; then
    ((count++))
fi

if [[ "src/verify.c" == *.c ]]; then
    ((count++))
fi

if [ $count -eq 4 ]; then
    echo "Sucessfully checked if source code is written in C"
    result=0
else
    result=1
fi


exit $result