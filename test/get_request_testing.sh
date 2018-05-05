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

echo "Testing GET request of zeroResponse file"
http_proxy="blue.cs.sonoma.edu:$1" wget blue.cs.sonoma.edu/~gkaiser/zeroResponse -q --prefer-family=IPv4 -O proxy_return/zeroResponse.ret
diff -q proxy_return/zeroResponse.ret check_values/zeroResponse.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "zeroResponse Test Passed"
else
    echo "zeroResponse Test Failed"
fi

echo "Testing a Large Request"
http_proxy="blue.cs.sonoma.edu:$1" wget blue.cs.sonoma.edu/~nwalker/bigworld.png -q --prefer-family=IPv4 -O proxy_return/bigworld.ret
diff -q proxy_return/bigworld.ret check_values/bigworld.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "Large Request Test Passed"
else
    echo "Large Request Test Failed"
fi