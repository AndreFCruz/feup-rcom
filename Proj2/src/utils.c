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

/*
struct hostent {
    char    *h_name;	Official name of the host.
    char    **h_aliases;	A NULL-terminated array of alternate names for the host.
    int     h_addrtype;	The type of address being returned; usually AF_INET.
    int     h_length;	The length of the address in bytes.
    char    **h_addr_list;	A zero-terminated array of network addresses for the host.
    Host addresses are in Network Byte Order.
};

#define h_addr h_addr_list[0]	The first address in h_addr_list.
*/
    if ((h=gethostbyname(domain)) == NULL) {
        herror("gethostbyname");
        exit(1);
    }

    /*
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));
    */

    return inet_ntoa(*((struct in_addr *)h->h_addr));
}

int logError(char * msg) {
  printf("%s %s\n", "ERROR:", msg);
  return FALSE;
}
