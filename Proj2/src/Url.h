#ifndef __URL_H
#define __URL_H

#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

typedef char url_content[256];

typedef struct url_t {
  int port; // integer to port
  url_content ip; // string to IP
  url_content host; // string to host
  url_content path; // string to path
  url_content filename; // string to filename
  url_content user; // string to user
  url_content password; // string to password
} URL;

void constructURL();
int parseURL(URL* url, const char* str); // Parse a string with the url to create the URL structure
void destructURL();

#endif /* __URL_H */