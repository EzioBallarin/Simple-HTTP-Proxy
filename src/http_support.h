/*
* Header for support functions, used to build a HTTP 1.0 compliant webserver
* Courtesy of sample-webserver-ex
*/

#ifndef _HTTP_SUPPORT_H_
#define _HTTP_SUPPORT_H_
#include <stdio.h>

// seconds to timeout waiting for activity
#define CLIENT_TIMEOUT    (5)

// listen() backlog
#define LISTEN_QUEUE      (10)

// buffer large enough to read in a request
#define MAX_REQUEST_LEN   (1024)

// default port for server
#define DEFAULT_PORT        "48494"

// default server root
#define DEFAULT_SERVER_ROOT "/home/student/eballarin/cs365/sample-webserver-ex/"

//
// Status codes from RFC 1945 (6.1.1)
//
#define OK              200
#define CREATED         201
#define ACCEPTED        202
#define NO_CONTENT      204
#define MOVED_PERM      301
#define MOVED_TEMP      302
#define NOT_MODIFIED    304
#define BAD_REQUEST     400
#define UNAUTHORIZED    401
#define FORBIDDEN       403
#define NOT_FOUND       404
#define INTERNAL_ERROR  500
#define NOT_IMPLEMENTED 501
#define BAD_GATEWAY     502
#define UNAVAILABLE     503

typedef struct {
    char* uri;
    int port;
    char* host;
    char* user_agent;
    struct http_header* other_headers; // Linked list of other headers in request
} http_req;

typedef struct {
    char* header;
    char* value;
    struct http_header* next;
} http_header;

//
// Function prototypes
//
void init_req(http_req *req);
void free_req(http_req *req);
void print_req(http_req *req);
void parse_client_request(char* req, http_req* req_fields);
void parse_client_request_headers(char* headers, http_req* req_fields);
void send_client_request(int client_socket, http_req* req_fields);
void exit_msg(int cond, const char* msg);


// a helpful debug macro
//   you can use it by calling debug_here();
#define debug_here(a)  \
do { \
    printf("%d: at %s:%d\n", getpid(), __FILE__, __LINE__); \
    fflush(stdout); \
} while(0)

#endif // _HTTP_SUPPORT_H_
