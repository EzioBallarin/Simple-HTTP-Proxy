// Courtesy of sample-webserver-ex
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <netdb.h>
#include <getopt.h>

#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

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
    // Setup the fields of an HTTP request
    // Our dummy request will go for google's home page.
    char client_request[2048];
    char client_request_resource[] = "/";
    char client_request_url[] = "www.google.com";
    sprintf(client_request,
        "GET %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n",
        client_request_resource, client_request_url
    );
    printf(client_request);

    // Set up an addrinfo struct and an addrinfo linked list
    // The hints addrinfo provides the structure which getaddrinfo()
    // should look for in its result linked list
    // The *infptr is the head of the result linked list retrieved
    // by getaddrinfo()
    struct addrinfo hints, *infoptr;

    // Set the family of addresses to look at to be IPV4
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;     // IPV4 records only
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets

    // Param 1 = the host name to look up
    // Param 2 = a service to look up (left NULL since
    // we don't know the port)
    // Param 3 = pointer to the struct which will be the guideline for
    // returned records
    // Param 4 = pointer to the head pointer of the linked list of results
    int request_addr_info = getaddrinfo(
        client_request_url,
        "http", &hints, &infoptr
    );

    // Checks if the return value was nonzero
    // According to the manpage, a nonzero value from getaddrinfo()
    // indicates an error occurred.
    if (request_addr_info) {
        fprintf(
            stderr, "getaddrinfo: %s\n", gai_strerror(request_addr_info)
        );
        exit(0);
    }

    // The proxy now acts as a client by opening a new socket
    // and using the address extracted from getaddrinfo()
    // as the destination for the write() of the http request
    // on behalf of the connected client
    struct addrinfo *record;
    char host[256];
    for (record = infoptr; record != NULL; record = record->ai_next) {

        socklen_t addr_size = 0;
        struct sockaddr* client_req_addr;
        if (record->ai_family == AF_INET) {
            addr_size = INET_ADDRSTRLEN;
            client_req_addr = (struct sockaddr_in*) record->ai_addr;
        } else {
            addr_size = INET6_ADDRSTRLEN;
            client_req_addr = (struct sockaddr_in6*) record->ai_addr;
        }

        char addr_string[addr_size];

        if (inet_ntop(
                record->ai_family, client_req_addr, addr_string, addr_size
            ) == NULL ) {
            fprintf(stderr, "inet_ntop: %s\n", strerror(errno));
            continue;
        }
        getnameinfo(
            record->ai_addr, record->ai_addrlen, host, sizeof(host), 
            NULL, 0, NI_NUMERICHOST
        );

        printf("Checking %s\t%s...\n", addr_string,host);
        int req_socket = socket(
            record->ai_family, record->ai_socktype, record->ai_protocol
        );
        if (req_socket == -1) {
            fprintf(stderr, "req_socket: %s\n", strerror(errno));
            continue;
        } 

        printf("Socket created for request to google...\n");

        int client_req_conn = connect(
            req_socket, client_req_addr, record->ai_addrlen
        );
        if (client_req_conn == -1) {
            fprintf(stderr, "client_req_conn: %s\n", strerror(errno));
            continue;
        }

        printf("Successfully connected to google...\n");

        int client_req_write = write(
            req_socket, client_request, sizeof(client_request)
        );
        if (client_req_write == -1) {
            fprintf(stderr, "client_req_write: %s\n", strerror(errno));
            continue;
        }

        printf("Request sent...\n");

        char remote_response[1024];
        int client_req_read = 0;
        int client_req_resp = 0;
        printf("Receiving response...\n");
        while ((client_req_read = read(
            req_socket, &remote_response, sizeof(remote_response))) > 0) {

            printf("sending chunk...\n");
            client_req_resp = write(
                client_socket, &remote_response, sizeof(remote_response)
            );
            if (client_req_resp == -1) {
                fprintf(
                    stderr, "client_req_resp: write(): %s\n", strerror(errno)
                );
                exit(1);
                break;
            }

        }

        if (client_req_read == -1) {
            fprintf(stderr, "client_req_read: %s\n", strerror(errno));
            continue;
        }
        printf("Response transmitted to client...\n\n");
        break; 

    }
    if (record == NULL) {
        fprintf(stderr, "failed to connect to %s\n", client_request_url);
        exit(1);
    }


    // Free up any memory allocated for the linked list of results
    freeaddrinfo(infoptr);

}
