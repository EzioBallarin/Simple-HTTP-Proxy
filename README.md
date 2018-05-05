# CS365-Lab3

Automated Tests:
1) Tests requesting a binary file.
2) Tests requesting an image file.
3) Tests requesting an HTML file
4) Tests requesting a file of size zero.
5) Tests requesting a large file(~200MB).

To run the automated tests, change to the "test"
directory. Run `./get_request_testing.sh [port
number proxy is running on]`

It will generate non-proxied "test files" in
which the proxied return will be tested against.