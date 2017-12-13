#include <regex.h>
#include "URL.h"

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

void destructURL(URL * url) {
  free(url);
}

int parseURL(URL * url, const char* str) {

}