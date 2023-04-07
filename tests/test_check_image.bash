#!/bin/bash
echo "Check for Image"

if [ -f "test.image" ]; then
    echo "Successfully found Image File"
    result=0
else   
    result=1
fi

exit $result