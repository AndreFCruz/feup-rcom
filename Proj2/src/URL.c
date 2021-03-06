#include <string.h>
#include <regex.h>
#include "URL.h"
#include "utils.h"

#define URL_REGEX "^ftp://(([a-zA-Z][^:]*):([^@]+)@)?(([a-z0-9:]+[.]?)+)/(([^/]+[/])*)([^/]+)$"

static int setInUrl(URL * url, int idx, const char * src, int size) {
  switch (idx) {
    case 0: // whole capture
    case 1: // identity:password
    case 5: // host's country
    case 7: // path termination
      break;
    case 2: // username
      if (0 == size) {
        strcpy(url->username, "anonymous");
      } else {
        strncpy(url->username, src, size);
        url->username[size] = 0;
      }
      break;
    case 3: // password
      if (0 == size) {
        strcpy(url->password, "a");
      } else {
        strncpy(url->password, src, size);
        url->password[size] = 0;
      }
      break;
    case 4: // hostname
      strncpy(url->hostname, src, size);
      url->hostname[size] = 0;
      break;
    case 6: // path
      strncpy(url->path, src, size);
      url->path[size] = 0;
      break;
    case 8: // path
      strncpy(url->filename, src, size);
      url->filename[size] = 0;
      break;
    default:
      break;
  }

  return OK;
}

/*
 * Match the string in "to_match" against the compiled regular
 * expression in "r", storing the values in the given URL*.
 */
static int match_url_regex(regex_t * r, const char * to_match, URL * url) {
  /* "p" is a pointer into the string which points to the end of the
     previous match. */
  const char * p = to_match;

  /* "N_MATCHES" is the maximum number of matches allowed. */
  const int N_MATCHES = 10;

  /* "m" contains the matches found. */
  regmatch_t m[N_MATCHES];

  int i = 0;
  while (i++ < N_MATCHES) {
    int j = 0;
    int nomatch = regexec(r, p, N_MATCHES, m, 0);
    if (nomatch) {
        return i == 1 ? NO_MATCH : OK;
    }
    for (j = 0; j < N_MATCHES; j++) {
      int start = m[j].rm_so + (p - to_match);
      int finish = m[j].rm_eo + (p - to_match);

      setInUrl(url, j, to_match + start, finish - start);
    }
    p += m[0].rm_eo;
  }

  return 0;
}

int parseURL(URL * url, const char* str) {
  regex_t r;
  const char * regex_text = URL_REGEX;
  int ret = 0;

  ret |= compile_regex(&r, regex_text);
  ret |= match_url_regex(&r, str, url);
  regfree(&r);

  return ret;
}

void printURL(URL * url) {
  printf("\nContents of URL struct:\n");
  printf("ip: %s\n", url->ip);
  printf("path: %s\n", url->path);
  printf("hostname: %s\n", url->hostname);
  printf("filename: %s\n", url->filename);
  printf("username: %s\n", url->username);
  printf("password: %s\n\n", url->password);
}

URL * constructURL() {
  URL * url = malloc(sizeof(URL));

  memset(url->ip, 0, URL_STR_LEN);
  memset(url->path, 0, URL_STR_LEN);
  memset(url->hostname, 0, URL_STR_LEN);
  memset(url->filename, 0, URL_STR_LEN);
  memset(url->username, 0, URL_STR_LEN);
  memset(url->password, 0, URL_STR_LEN);
  url->port = 21;

  return url;
}

void setIp(URL * url){
    memset(url->ip, 0, URL_STR_LEN);
    strcpy(url->ip, getIp(url->hostname));
}

void destructURL(URL * url) {
  free(url);
}