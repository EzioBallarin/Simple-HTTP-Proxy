# Simple HTTP Proxy

Ezio Ballarin, Jack Newman

Design:
We took the classic approach laid out by Beej as our guiding light when 
designing this proxy. For our purposes, we only needed a barebones 
client/server within the proxy, so we also took inspiration from the 
given sample code for the client-server-ex and webserver-ex code. 

To implement HTTP 1.0 we envisioned our proxy as a "pass through" server
which did not manipulate headers or responses, but just let them flow
through untouched. Meaning, user-agents were not altered, keep-alive
headers were maintained (even though we don't support that, since that's
HTTP 1.1), and responses were not examined before sent back to the client.

We did create our own struct for containing the individual pieces 
of a received HTTP request from a connected user. This struct contained
fields for the port, URI, host, and headers for the request. Using 
this struct made for much easier parsing and reconstruction of the proxied
request from the proxy's perspective. 

The read request buffer is 2048 bytes, and the write response buffer is 1
byte. The read request buffer is large enough to handle most typical 
sized GET requests, but it may not catch all headers in every case. Given
the scope of this project, we felt that losing headers would be preferential
to not even servicing a request simply because of headers. The write
response buffer is 1 byte due to simplicity. This impacts performance,
but it ensures each and every byte from the destination host is sent 
to the client, in order. During testing there were issues with having
a larger buffer size, typically because the Javascript would turn into
plaintext and would just be another elemnt on the screen, rather than
executed in a browser, or left inside of the <script></script> tags. 

Testing was both manual and automated. All automated tests were first 
performed manually with
 * The proxy running on localhost port 8001
 * The proxy running on localhost port 14441
 * The proxy running on our school's CS department server, ports 8001,
   14441, 8150, 8199, 8002, etc. 

After verifying our tests manually, we bundled them into a script
for ease of use. 

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
