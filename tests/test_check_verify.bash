#!/bin/bash
echo "Test to check verify input file"

./extfat -v 

if [ $? -eq 1 ]; then
    echo "Successfully checked for an input file"
    result=0
else
    result=1
fi


exit $result