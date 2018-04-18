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

    // Construct an IPV4 sockaddr that binds to all interfaces
    // (since we specify s_addr = INADDR_ANY) on the given port 
    // by the user
    struct sockaddr_in proxy_addr;
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_port = htons(proxy_port);
    proxy_addr.sin_addr.s_addr = INADDR_ANY;


    // Bind the socket to the user's given port on all IP addresses 
    // the machine running the proxy has
    int proxy_bind = bind( 
        proxy_socket, 
        (struct sockaddr*) &proxy_addr, 
        sizeof(proxy_addr)
    );
    if (proxy_bind == -1) {
        fprintf(stderr, "bind() failed: %s\n", strerror(errno));
        return -1;
    }   

    // Set the socket to listen on the user's given port
    int proxy_listen = listen(proxy_socket, LISTEN_QUEUE);
    if (proxy_listen == -1) {
        fprintf(stderr, "listen() failed: %s\n", strerror(errno));
    }

    // Handle incoming connections in the listen() queue on the proxy socket
    int client_socket;
    while (1) {

        // Setup sockaddr to store connecting client's address
        struct sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);

        // Accept the connection; put the client's address info
        // in the sockaddr_in struct setup above
        client_socket = accept(
            proxy_socket, 
            (struct sockaddr*) &client_addr, 
            &client_addr_size
        );
        if (client_socket == -1) {
            fprintf(stderr, "accept() failed: %s\n", strerror(errno));
            return -1;
        }

        /********************* TEST CODE ********************** */
        // Setup a string for the IP address to go
        char client_ip_address[INET_ADDRSTRLEN];
        // Convert the client's sockaddr to an IPV4 readable string
        // and put it into the client_ip_address string defined above
        if (inet_ntop(
                AF_INET, 
                &(client_addr.sin_addr), 
                client_ip_address, 
                INET_ADDRSTRLEN
            ) == NULL) {
            fprintf(stderr, "inet_ntop() failed: %s\n", strerror(errno));
            return -1;
        }
        printf("Client %s connected...\n", client_ip_address); 
        /********************* TEST CODE ********************** */


        /* Entering the real meat of the netcode */
        // Setup a receiving buffer to carry the message sent from the client
        char client_request[MESSAGE_BUFFER_LEN];

        // Receive the data from the client socket, 
        // place it in the request buffer
        int client_recv = recv(
            client_socket, 
            &client_request, 
            MESSAGE_BUFFER_LEN, 
            0
        );
        if (client_recv == -1) {
            fprintf(stderr, "recv() failed: %s\n", strerror(errno));
            return -1;
        }

        // Print out the client's request
        printf("%s\n", client_request); 
        
        // Setup an HTML response
        char html_resp[1024] = "HTTP/1.1 200 OKi \r\n\n <html>wow</html>";
        
        // Send the HTML response to the client socket
        int client_send =  send(
            client_socket, 
            html_resp, 
            sizeof(html_resp), 
            0
        );
        if (client_send == -1) {
            fprintf(stderr, "send() failed: %s\n", strerror(errno));
            return -1;
        }

        // Close the connection between the proxy socket and client socket
        int client_close = close(client_socket);
        if (client_close == -1) {
            fprintf(stderr, "close() failed: %s\n", strerror(errno));
            return -1;
        }
    }
    // Close the proxy down.
    int proxy_close = close(proxy_socket); 
    if (proxy_close == -1) {
        fprintf(stderr, "close() failed: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}
