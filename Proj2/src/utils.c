#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

#include "utils.h"

#define MAX_ERROR_MSG 0x1000

/* Compile the regular expression described by "regex_text" into "r". */
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

/*
  Match the string in "to_match" against the compiled regular
  expression in "r".
 */
int match_regex (regex_t * r, const char * to_match) {
  /* "P" is a pointer into the string which points to the end of the
     previous match. */
  const char * p = to_match;
  /* "N_matches" is the maximum number of matches allowed. */
  const int n_matches = 10;
  /* "M" contains the matches found. */
  regmatch_t m[n_matches];

  while (1) {
    int i = 0;
    int nomatch = regexec (r, p, n_matches, m, 0);
    if (nomatch) {
        printf ("No more matches.\n");
        return nomatch;
    }
    for (i = 0; i < n_matches; i++) {
      int start;
      int finish;
      if (m[i].rm_so == -1) {
        break;
      }
      start = m[i].rm_so + (p - to_match);
      finish = m[i].rm_eo + (p - to_match);
      if (i == 0) {
        printf ("$& is ");
      }
      else {
        printf ("$%d is ", i);
      }
      printf ("'%.*s' (bytes %d:%d)\n", (finish - start),
              to_match + start, start, finish);
    }
    p += m[0].rm_eo;
  }
  return 0;
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
