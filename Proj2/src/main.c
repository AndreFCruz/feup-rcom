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
#include "utils.h"

#define SERVER_PORT 21
#define SERVER_ADDR "192.168.28.96"
#define BUFFER_SIZE 1024

void printUsage(char* argv0) {
	printf("\nUsage1 Normal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",
			argv0);
	printf("Usage2 Anonymous: %s ftp://<host>/<url-path>\n\n", argv0);
}

int main(int argc, char** argv){
  regex_t r;
  const char * regex_text = "^ftp:\/\/(([a-zA-Z][a-zA-Z0-9]*):([a-zA-Z0-9]+)@)?(([a-zA-Z][a-zA-Z0-9]*[.]?)+)\/(([^\/\s]+\/)*([^\/\s]+))?$";
  const char * find_text = argv[1];

  printf ("Trying to find '%s' in '%s'\n", regex_text, find_text);
  compile_regex(& r, regex_text);
  match_regex(& r, find_text);
  regfree(& r);

  return 0;
}
