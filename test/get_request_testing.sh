#! /bin/bash

#use first cli for port number.

# readd this to front of all wgets: http_proxy="blue.cs.sonoma.edu:$1" 

echo "Testing GET request of binary file"
http_proxy="blue.cs.sonoma.edu:$1" wget --input-file=inputs/binaryIn -q --prefer-family=IPv4 -O proxy_return/bin.ret
diff -q proxy_return/bin.ret check_values/binary.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "Binary Test Passed"
else
    echo "Binary Test Failed"
fi

echo "Testing GET request of image file"
http_proxy="blue.cs.sonoma.edu:$1" wget --input-file=inputs/imageIn -q --prefer-family=IPv4 -O proxy_return/pictureTest.ret
diff -q proxy_return/pictureTest.ret check_values/image.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "Image Test Passed"
else
    echo "Image Test Failed"
fi

echo "Testing GET request of HTML file"
http_proxy="blue.cs.sonoma.edu:$1" wget --input-file=inputs/HTMLIn -q --prefer-family=IPv4 -O proxy_return/test.ret
diff -q proxy_return/test.ret check_values/HTML.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "HTML Test Passed"
else
    echo "HTML Test Failed"
fi