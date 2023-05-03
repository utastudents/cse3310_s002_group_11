#!/bin/bash
echo "Test to check if input file matches output file"

if [ -f test2.image ]; then
    rm test2.image
fi

# Note, not implemented at this time..just a cut and paste...

result1=($(/usr/bin/md5sum test.image))

# this should be a call to extfat -c <infile> <outfile>

./extfat -c -i test.image -o test2.image

 
result2=($(/usr/bin/md5sum test2.image))

# clean up
$(rm test2.image)
 
if [ "${result1[0]}" = "${result2[0]}" ]; then
    echo "Successfully copied input file to output file"
    result=0
else
    result=1
fi
exit $result