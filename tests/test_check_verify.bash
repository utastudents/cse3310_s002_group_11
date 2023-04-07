#!/bin/bash
echo "Test to check verify input file"

./extfat -v -i test.image

if [ $? -eq 0 ]; then
    echo "Successfully verified input file"
    result=0
else
    result=1
fi


exit $result