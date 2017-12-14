#include <regex.h>
#include "URL.h"
#include "utils.h"

int parseURL(URL * url, const char* str) {
  // regex_t r;
  // const char * regex_text;
  // const char * find_text;
  // if (argc != 3) {
  //   regex_text = "([[:digit:]]+)[^[:digit:]]+([[:digit:]]+)";
  //   find_text = "This 1 is nice 2 so 33 for 4254";
  // }
  // else {
  //   regex_text = argv[1];
  //   find_text = argv[2];
  // }
  // printf ("Trying to find '%s' in '%s'\n", regex_text, find_text);
  // compile_regex (& r, regex_text);
  // match_regex (& r, find_text);
  // regfree (& r);
  return 0;
}

URL * constructURL() {
  URL * url = malloc(sizeof(URL));
  memset(url->user, 0, URL_STR_LEN);
  memset(url->password, 0, URL_STR_LEN);
  memset(url->host, 0, URL_STR_LEN);
  memset(url->path, 0, URL_STR_LEN);
  memset(url->filename, 0, URL_STR_LEN);
  url->port = 21;

  return url;
}

void setURLTestValues(URL * url) {
  strcpy(url->user, "anonymous");
  strcpy(url->password, "mail@domain");
  strcpy(url->host, "ftp.dei.uc.pt");
  strcpy(url->path, "pub/linux/CentOS/");
  strcpy(url->filename, "timestamp.txt");

}

void fillIp(URL * url){
    memset(url->ip, 0, URL_STR_LEN);
    strcpy(url->ip, getIp(url->host));
}

void destructURL(URL * url) {
  free(url);
}
