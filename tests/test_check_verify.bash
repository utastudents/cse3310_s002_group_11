#!/bin/bash
echo "Test to check verify input file"

if [ -f test2.image ]; then
    rm test2.image
fi

./extfat -c -i test.image -o test2.image

result1=($(./extfat -v -i test.image))
result2=($(./extfat -v -i test2.image))

$(rm test2.image)

if [ "${result1[0]}" = "${result2[0]}" ]; then
    echo "Successfully verified input file"
    result=0
else
    result=1
fi


exit $result