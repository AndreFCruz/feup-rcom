#ifndef __URL_H
#define __URL_H

#define URL_STR_LEN 256

#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef struct url_t {
  int port;       // integer to port
  char ip[URL_STR_LEN];   // string to IP
  char host[URL_STR_LEN]; // string to host
  char path[URL_STR_LEN]; // string to path
  char filename[URL_STR_LEN]; // string to filename
  char user[URL_STR_LEN]; // string to user
  char password[URL_STR_LEN]; // string to password
} URL;

/**
 * URL struct constructor
 */
URL * constructURL();

/**
 * Parses the url command to the given URL struct
 */
int parseURL(URL* url, const char* str);

/**
 * URL struct destructor
 */
void destructURL(URL * url);

#endif /* __URL_H */