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
  int port;
  char ip[URL_STR_LEN];       // host's ip
  char path[URL_STR_LEN];     // file path
  char hostname[URL_STR_LEN]; // hostname
  char filename[URL_STR_LEN]; // filename
  char username[URL_STR_LEN]; // username
  char password[URL_STR_LEN]; // password
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
 * Sets the ip field from the URL struct
 */
void setIp(URL * url);

/**
 * Prints the contents of the URL struct to STDOUT
 */
void printURL(URL * url);

/**
 * URL struct destructor
 */
void destructURL(URL * url);

#endif /* __URL_H */