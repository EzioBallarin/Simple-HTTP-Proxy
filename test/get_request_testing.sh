#! /bin/bash

#use first cli for port number.

mkdir -p check_values proxy_return

echo "Generating binary test file"
wget --input-file=inputs/binaryIn -q --prever-family=IPv4 -O check_values/binary.check
echo "Testing GET request of binary file"
http_proxy="blue.cs.sonoma.edu:$1" wget --input-file=inputs/binaryIn -q --prefer-family=IPv4 -O proxy_return/bin.ret
diff -q proxy_return/bin.ret check_values/binary.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "Binary Test Passed"
else
    echo "Binary Test Failed"
fi

echo "Generating image test file"
wget --input-file=inputs/imageIn -q --prefer-family=IPv4 -O proxy_return/pictureTest.check
echo "Testing GET request of image file"
http_proxy="blue.cs.sonoma.edu:$1" wget --input-file=inputs/imageIn -q --prefer-family=IPv4 -O proxy_return/pictureTest.ret
diff -q proxy_return/pictureTest.ret check_values/image.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "Image Test Passed"
else
    echo "Image Test Failed"
fi

echo "Generating HTML test file"
wget --input-file=inputs/HTMLIn -q --prefer-family=IPv4 -O proxy_return/HTML.check
echo "Testing GET request of HTML file"
http_proxy="blue.cs.sonoma.edu:$1" wget --input-file=inputs/HTMLIn -q --prefer-family=IPv4 -O proxy_return/HTML.ret
diff -q proxy_return/HTML.ret check_values/HTML.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "HTML Test Passed"
else
    echo "HTML Test Failed"
fi

echo "Generating zero response test file"
wget blue.cs.sonoma.edu/~gkaiser/zeroResponse -q --prefer-family=IPv4 -O proxy_return/zeroResponse.check
echo "Testing GET request of zeroResponse file"
http_proxy="blue.cs.sonoma.edu:$1" wget blue.cs.sonoma.edu/~gkaiser/zeroResponse -q --prefer-family=IPv4 -O proxy_return/zeroResponse.ret
diff -q proxy_return/zeroResponse.ret check_values/zeroResponse.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "zeroResponse Test Passed"
else
    echo "zeroResponse Test Failed"
fi

echo "Generating Large request test file"
wget blue.cs.sonoma.edu/~nwalker/bigworld.png -q --prefer-family=IPv4 -O proxy_return/bigworld.check
echo "Testing a Large Request"
http_proxy="blue.cs.sonoma.edu:$1" wget blue.cs.sonoma.edu/~nwalker/bigworld.png -q --prefer-family=IPv4 -O proxy_return/bigworld.ret
diff -q proxy_return/bigworld.ret check_values/bigworld.check > /dev/null 2>&1
if [ $? -eq 0 ]
then
    echo "Large Request Test Passed"
else
    echo "Large Request Test Failed"
fi