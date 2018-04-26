// Courtesy of sample-webserver-ex
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>
#include "http_support.h"

#define HTTP_TIME_FORMAT "%a, %d %b %Y %H:%M:%S GMT"
#define DIRS_TIME_FORMAT "%d-%b-%Y %H:%M:%S"

//
// initializes a request to sensible defaults
//
void init_req(http_req *req) {
    req->resource = NULL;
    req->mime = NULL;
    req->resource_fd = -1;
    req->method = UNSUPPORTED;
    req->status = OK;
    req->type = SIMPLE;
    req->length = -1;
}

//
// Free the resources associated with a request
//
void free_req(http_req *req) {
    if (req->resource_fd != -1) {
        close(req->resource_fd);
        req->resource_fd = -1;
    }
    if (req->resource) {
        free(req->resource);
        req->resource = NULL;
    }
}

/**
 * Name: print_req()
 * 
 * Purpose: Prints an HTTP request's headers. Reconstructs the full request
 *          as a string for debugging.
 * Parameters: http_req* req - the HTTP request struct to print
 * Return: None.
 * 
 */
void print_req(http_req* req) {

}

//
// A simple function to print error and exit, if cond is true
//
void exit_msg(int cond, const char* msg) {
    if (cond) {
        perror(msg);
        exit(-1);
    }
    return;
}

/**
 * Name: parse_client_request()
 * 
 * Purpose: Send the client's original request through the proxy
 * Parameters: int client_socket - the client FD socket
 *             char* req - the raw string HTTP request
 *             http_req* req_fields - pointer to the struct that will hold
 *                                    parsed individual headers
 * Return: None. 
 * 
 */
void parse_client_request(int client_socket, char* req, http_req* req_fields) {
    // TODO: parse char* req HTTP request for its header fields 
    // and recombine to create a proxied request
    
    printf("\tIN HELPER:\n%s\n", req);
    return;
}

/**
 * Name: send_client_request()
 * 
 * Purpose: Sends a parsed HTTP request on behalf of a client 
 * Parameters: int client_socket - the client who sent the proxy a request
 *             http_req* req_fields - the parsed HTTP request
 * Return: None
 * 
 */
void send_client_request(int client_socket, http_req* req_fields) {

}
