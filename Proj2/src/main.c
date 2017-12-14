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

#define BUFFER_SIZE 1024

void printUsage(char ** argv) {
	printf(
    "\nUsage: %s ftp://[<user>:<password>@]<host>/<url-path>\n",
		argv[0]);
}

int main(int argc, char** argv) {

  if (argc != 2) {
    printUsage(argv);
    exit(1);
  }

  downloadFtpUrl(argv[1]);

  return 0;
}