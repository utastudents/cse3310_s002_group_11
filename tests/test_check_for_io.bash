#!/bin/bash
echo "Test to check an input and output file name is given"

./extfat -c

if [ $? -eq 1 ]; then
    echo "Successfully checked for an input and output file"
    result=0
else
    result=1
fi


exit $result