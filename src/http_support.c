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

char *server_root = DEFAULT_SERVER_ROOT;

//
// initializes a request to sensible defaults
//
void init_req(http_req *req)
{
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
void free_req(http_req *req)
{
    if (req->resource_fd != -1) {
        close(req->resource_fd);
        req->resource_fd = -1;
    }
    if (req->resource) {
        free(req->resource);
        req->resource = NULL;
    }
}

//
// Print out a requst
//
void print_req(http_req *req) {
    char req_method_str;
    if (req->method == GET) {
        strcpy(&req_method_str, "GET");
    }
    printf("Here is the request:\n");
    printf("%s %s %s\n", req_method_str, req->resource, req->mime);
}

//
// Reads in all the headers sent by the client
// and uses them to populate req
//
int http_get_request(FILE *stream, http_req *req)
{
    int len, rval, lineno, fd;
    char *ptr, *val, buf[MAX_REQUEST_LEN];
    fd_set fds;
    struct timeval tv;

    // initialize
    fd = fileno(stream);
    tv.tv_sec = CLIENT_TIMEOUT;
    tv.tv_usec = 0;
    lineno = 0;

    // Just get the first line of the request. The
    //  first line contains the essential URI request,
    //  and contains enough information to distinguish between
    //  full and simple requests.
    //
    // Simple-Request = "GET" SP Request-URI CRLF
    // Full-Request =   Method SP Request-URI SP HTTP-Version CRLF
    //                  *(General-Header | Request-Header | Entity-Header)
    //                  CRLF
    //                  [Entity-Body]
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    rval = select(fd + 1, &fds, NULL, NULL, &tv);
    exit_msg(rval < 0, "select() error");
    if (rval == 0) return -1;

    fgets(buf, sizeof(buf), stream);
    ptr = buf;
    lineno++;

    // "GET" SP Request-URI CRLF
    // Method SP Request-URI SP HTTP-Version CRLF
    if (lineno == 1) {
        if (strncmp(ptr, "GET ", 4) == 0) {
            req->method = GET;
            ptr += 4;
        } else if (strncmp(ptr, "HEAD ", 5) == 0) {
            req->method = HEAD;
            ptr += 5;
        } else {
            req->method = UNSUPPORTED;
            req->status = NOT_IMPLEMENTED;
            return -1;
        }
        // skip SP
        while (*ptr && isspace(*ptr))
            ptr++;
        // Request-URI
        val = strchr(ptr, ' ');
        if (!val)
            len = strlen(ptr);
        else
            len = val - ptr;
        if (len < 1) {
            req->status = BAD_REQUEST;
            return -1;
        }
        req->resource = malloc(len+1);
        if (req->resource == NULL) {
            req->status = INTERNAL_ERROR;
            return -1;
        }
        strncpy(req->resource, ptr, len);
        req->resource[len] = '\0'; // make sure its null terminated

        // HTTP-Version present?
        if (strstr(ptr, "HTTP/"))
            req->type = FULL;
        else
            req->type = SIMPLE;
    } else {
        // we are going to ignore the value of subsequent headers
        // but this is where we would process them
    }
    return 0;
}


//
// Sends a response to the client
//
// Based on RFC1945 (6)
// Response =           Simple-Response | Full-Response
// Simple-Response =    [Entity-Body]
// Full-Response =      Status-Line
//                      *(General-Header | Response-Header | Entity-Header)
//                      CRLF
//                      [Entity-Body]
//
int http_response(FILE *stream, http_req *req)
{
    int i, fd = fileno(stream);
    char c, timestr[128];
    time_t now;

    // (3.3) HTTP-date format
    now = time(NULL);
    strftime(timestr, sizeof(timestr), HTTP_TIME_FORMAT, gmtime(&now));

    // Send HTTP headers
    //  Only full request need an HTTP header in the response
    //
    // Status-Line
    // *(General-Header | Response-Header | Entity-Header)
    // CRLF
    if (req->type == FULL) {
        fprintf(stream, "HTTP/1.0 %d\r\n", req->status);
        fprintf(stream, "Server: CS2140 v1\r\n");
        fprintf(stream, "Date: %s\r\n", timestr);
        if ((req->status == OK) || (req->status == NO_CONTENT))
            fprintf(stream, "Content-Length: %d\r\n", req->length);
        fprintf(stream, "Content-Type: %s\r\n", 
            (req->mime && (req->status == OK)) ? req->mime : "text/html");
        fprintf(stream, "\r\n");
        fflush(stream);
    }

    // Send Entity-Body
    //
    if (req->method == HEAD) {
        return 0;
    } else if (req->status == OK) {
    // read the resource_fd; write it out to the client
        lseek(req->resource_fd, 0, SEEK_SET);
        while ((i = read(req->resource_fd, &c, 1))) {
            write(fd, &c, 1);
        }
    } else {
    // Return an error message
        fprintf(stream, "<HTML>\n<HEAD>\n<TITLE>");
        fprintf(stream, "Server Error %d", req->status);
        fprintf(stream, "</TITLE>\n</HEAD>\n<BODY>\n<H1>");
        fprintf(stream, "Server Error %d", req->status);
        fprintf(stream, "</H1>\n<P>");
        fprintf(stream, "The request could not be completed.");
        fprintf(stream, "</P>\n</BODY>\n</HTML>\n");
        fflush(stream);
    }
    return 0;
}


//
// Parses the request from the client and populates req
//
int http_process_request(http_req *req)
{
    int i, j, num;
    char path[PATH_MAX], pathbuf[PATH_MAX], timestr[32];
    char *fullpath = NULL;
    struct tm *tp;
    struct dirent *de, **dir;
    struct stat statbuf;
    FILE *f = NULL;
    char template[] = "/tmp/wserver.XXXXXX";

    if (req->method != GET)
        return 0;

    // path with server_root prepended
    memset(path, '\0', sizeof(path));
    strncpy(path, server_root, sizeof(path));
    if (req->resource[0] != '/')
        strncat(path, "/", sizeof(path));
    strncat(path, req->resource, sizeof(path) - strlen(path));

    // check that the requested path falls under the server root
    //   get rid of possible relative path trickery
    if ((fullpath = realpath(path, fullpath)) == NULL) {
        req->status = NOT_FOUND;
    } else if (strncmp(path, server_root, strlen(server_root)) != 0) {
        req->status = NOT_FOUND;
    }
    if (fullpath)
        free(fullpath);

    // if we have an error so far, don't bother continuing
    if (req->status != OK) {
        return 0;
    }

    // stat it -- could we open it read-only?
    if (stat(path, &statbuf) < 0) {
        // handle not found and not authorized conditions
        if (errno == EACCES)
            req->status = UNAUTHORIZED;
        else
            req->status = NOT_FOUND;
        return 0;
    } else if (S_ISDIR(statbuf.st_mode)) {
    // its a directory
        if (path[strlen(path)] != '/') {
            strncat(path, "/", sizeof(path) - strlen(path));
        }
        //  create temp file and put its fd into resource_fd
        //  fill it with a response, which will get sent to the client
        req->resource_fd = mkstemp(template);
        if (req->resource_fd > -1) {
            f = fdopen(dup(req->resource_fd), "r+");
            unlink(template);
        }
        if (f == NULL)  {
            req->status = INTERNAL_ERROR;
            return 0;
        }
        // the response will be a list of dir contents
        fprintf(f, "<HTML><HEAD><TITLE>Index of %s", req->resource);
        fprintf(f, "</TITLE></HEAD>\r\n<BODY>");
        fprintf(f, "<H2>Index of %s</H2>\r\n", req->resource);
        fprintf(f, "<PRE>\n");
        fprintf(f, "%-60s %-30s %s", "Name", "Last Modified", "Size");
        fprintf(f, "\r\n<HR>\r\n");

        // directory contents, alphabetically sorted
        num = scandir(path, &dir, 0, &alphasort);
        for (j = 0; j < num; j++) {
            de = dir[j];

            // full path, for stat
            strncpy(pathbuf, path, sizeof(pathbuf));
            strncat(pathbuf, de->d_name, sizeof(pathbuf));
            if (stat(pathbuf, &statbuf) < 0) {
            	free(de);
            	continue;
            }
            // modification time, as a string
            tp = gmtime(&statbuf.st_mtime);
            strftime(timestr, sizeof(timestr), DIRS_TIME_FORMAT, tp);

            // generate HTML respose content, line by line

            //   the link should have full path, not just d_name
            fprintf(f, "<A HREF=\"%s%s%s\">", req->resource,
                    de->d_name, S_ISDIR(statbuf.st_mode) ? "/" : "");
            fprintf(f, "%.59s%s",
                    de->d_name, S_ISDIR(statbuf.st_mode) ? "/" : " ");
            fprintf(f, "</A>");
            for(i = 0; i < 60 - strlen(de->d_name) - 1; i++)
                fprintf(f, " ");
            fprintf(f, "%-30s", timestr);
            if (!S_ISDIR(statbuf.st_mode)) {
                fprintf(f, " %ju", (uintmax_t) statbuf.st_size); 
            }
            fprintf(f, "\n");
            
            // remember to free each entry in dir
            free(de);
        }
        free(dir);

        fprintf(f, "</PRE>\r\n<HR>\r\n</BODY></HTML>\r\n");
        fclose(f);

        if (fstat(req->resource_fd, &statbuf) < 0) {
            // we wrote the file, so we should be able to fstat it
            req->status = INTERNAL_ERROR;
        } else {
            req->length = (int) statbuf.st_size;
        }
    } else {
    // its a file,
        //  open it and we will send it to the client
        req->resource_fd = open(path, O_RDONLY);
        if (req->resource_fd < 0) {
            if (errno == EACCES)
                req->status = UNAUTHORIZED;
            else
                req->status = INTERNAL_ERROR;
        } else {
            req->length = (int) statbuf.st_size;
            get_mime_type(req->resource, req);
        }
    }
    return 0;
}

//
// Sets the request's mime type to point to a string
// constant that is associated with the mime type;
// the server will generate a Content-Type HTTP header
// for the retrieved media.
//
int get_mime_type(char *filename, http_req *req)
{
    char *ext = strrchr(filename, '.');
    if (!ext)
        req->mime = NULL;
    else if (!strncmp(ext, ".html", 5) || !strncmp(ext, ".htm", 4))
        req->mime = "text/html";
    else if (!strncmp(ext, ".txt", 4))
        req->mime = "text/plain";
    else if (!strncmp(ext, ".c", 2) || !strncmp(ext, ".h", 2))
        req->mime = "text/plain";
    else if (!strncmp(ext, ".jpg", 4) || !strncmp(ext, ".jpeg", 5))
        req->mime = "image/jpeg";
    else if (!strncmp(ext, ".gif", 4))
        req->mime = "image/gif";
    else if (!strncmp(ext, ".png", 4))
        req->mime = "image/png";
    else if (!strncmp(ext, ".css", 4))
        req->mime = "text/css";
    else if (!strncmp(ext, ".au", 3))
        req->mime = "audio/basic";
    else if (!strncmp(ext, ".wav", 4))
        req->mime = "audio/wav";
    else if (!strncmp(ext, ".avi", 4))
        req->mime = "video/x-msvideo";
    else if (!strncmp(ext, ".mpeg", 5) || !strncmp(ext, ".mpg", 4))
        req->mime = "video/mpeg";
    else if (!strncmp(ext, ".mp3", 4))
        req->mime = "audio/mpeg";
    else
        req->mime = NULL;
    return 0;
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
