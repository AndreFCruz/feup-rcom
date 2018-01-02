#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"

#define MAX_ERROR_MSG 0x1000

/*
 * Compile the regular expression described by "regex_text" into "r".
 */
int compile_regex(regex_t * r, const char * regex_text) {
  int status = regcomp(r, regex_text, REG_EXTENDED|REG_NEWLINE);
  if (status != 0) {
  char error_message[MAX_ERROR_MSG];
  regerror (status, r, error_message, MAX_ERROR_MSG);
    printf ("Regex error compiling '%s': %s\n",
    regex_text, error_message);
    return 1;
  }
  return 0;
}

void insertCharAt(char * dest, const char * src, char c, int size, int idx) {
  if (idx >= size) return;
  strncpy(dest, src, size);
  memmove(dest + idx + 1, src + idx, size - idx);
  dest[idx] = c;
}

char * getIp(char * domain) {
  struct hostent *h;
  if ((h=gethostbyname(domain)) == NULL) {
      herror("gethostbyname");
      exit(1);
  }
  
  return inet_ntoa(*((struct in_addr *)h->h_addr));
}

int logError(char * errorMsg) {
  fprintf(stderr, "Error: %s\n", errorMsg);
  return ERROR;
}

void printDownloadProgress(int * dots) {
  (*dots)++;

  if(*dots == 3*DOWNLOAD_PROGRESS_RESET){
    *dots = 0;
    printf("\r                  ");
    printf("\rDownloading.");
  }
  else if(((*dots)%DOWNLOAD_PROGRESS_RESET) == 0)
    printf(".");

  fflush(stdout);
}