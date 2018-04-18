#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>


//
// Globals and constants
//
static int verbose = 1;
#define CRLF                "\r\n"
#define LISTEN_QUEUE        10
#define CLIENT_TIMEOUT      5
#define MESSAGE_BUFFER_LEN  2048

// Status codes from RFC 1945 (Section 6.1.1)
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


const struct option long_opts[] = {
    {"help", no_argument, 0, 'h'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
};

const char *opts_desc[] = {
    ", to display this usage message.",
    ", for verbose output.",
};

void usage(const char* arg)
{
    int i;
    printf("Usage: %s %s [OPTIONS], where\n", arg, "PORT");
    printf("  OPTIONS:\n");
    for (i = 0; i < sizeof(long_opts) / sizeof(struct option) - 1; i++) {
        printf("\t-%c, --%s%s\n",
            long_opts[i].val, long_opts[i].name, opts_desc[i]);
    }    
    exit(0);
}


//
// A simple function to print error and exit, if cond is true
//
void exit_msg(int cond, const char* msg)
{
    if (cond) {
        perror(msg);
        exit(-1);
    }
    return;
}



//
// Main
//
int main(int argc, char *argv[])
{
    int i, opt;
    char *port = NULL;

    // parse arguments
    while( -1 != (opt = getopt_long(argc, argv, "p:r:hv", long_opts, &i)) ) {
        switch(opt) {
            case 'h':
                usage(argv[0]);
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Invalid option.\n");
                usage(argv[0]);
                break;
        }
    } // while

    fflush(stdout);
    if (argc - optind != 1) {
        usage(argv[0]);
    } else {
        port = argv[optind];
    }
    
    
    if (verbose == 1) {
        printf("Using port [%s]\n", port); fflush(stdout);
    }
    
    // Parse the port string to a long
    char* end;
    long proxy_port = strtol(port, &end, 10);

    // Make a socket for the proxy to act
    // param 1 specifies IPV4 will be used 
    // param 2 specifies that it will be TCP traffic
    // param 3 sets the protocol to be the default protocol of IPV4 (TCP)
    int proxy_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (proxy_socket == -1) {
        fprintf(stderr, "socket() failed: %s\n", strerror(errno));
        return -1;
    }
    
    int optval = 1; 
    int proxy_opt = setsockopt(proxy_socket, SOL_SOCKET, SO_REUSEADDR,
                               &optval, sizeof optval);
    if (proxy_opt == -1) {
        fprintf(stderr, "setsockopt() failed: %s\n", strerror(errno));
        return -1;

    }

    // Construct a
    
    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(proxy_port);
    proxy_addr.sin_addr.s_addr = INADDR_ANY;
    


    bind(proxy_socket, (struct sockaddr*) &proxy_addr, sizeof(proxy_addr));
    listen(proxy_socket, 5);

    int client_socket;
    while (1) {
        client_socket = accept(proxy_socket, NULL, NULL);
        printf("Client connected...\n");
        send(client_socket, "HELLO", sizeof("HELLO"), 0);
        close(client_socket);
    }
    close(proxy_socket); 
    return 0;
}
