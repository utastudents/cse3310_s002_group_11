#!/bin/bash
echo "Test to check if input file name is same as output file"

./extfat -c -i test.image -o test.image

if [ $? -eq 0 ]; then
    echo "Successfully checked for matching input and output names"
    result=0
else
    result=1
fi


exit $result