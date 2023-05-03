#!/bin/bash
echo "Test to check an input and output file name is given"

if [ -f test2.image ]; then
    rm test2.image
fi

./extfat -c -i test.image -o test2.image

if [ $? -eq 0 ] && [ -f test2.image ]; then
    echo "Successfully checked for an input and output file"
    result=0
else
    result=1
fi

$(rm test2.image)

exit $result