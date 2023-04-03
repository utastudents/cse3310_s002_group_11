#!/bin/bash
echo "Check for Image"

if [ -f "test.image" ]; then
    echo "Image File exists"
    result=0
else   
    result=1
fi

exit $result