#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <regex.h>

#include "URL.h"
#include "utils.h"
#include "clientFTP.h"

#define SERVER_PORT 21
#define SERVER_ADDR "192.168.28.96"
#define BUFFER_SIZE 1024

void printUsage(char* argv0) {
	printf("\nUsage1 Normal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",
			argv0);
	printf("Usage2 Anonymous: %s ftp://<host>/<url-path>\n\n", argv0);
}

int main(int argc, char** argv) {

  URL * url = constructURL();

  if (argc == 2) {
    parseURL(url, argv[1]);
  }

  printURL(url);
  destructURL(url);

  return 0;
}