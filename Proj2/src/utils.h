#ifndef __UTILS_H
#define __UTILS_H

#include <sys/types.h>
#include <regex.h>

#define ERROR	1
#define OK  	0

#define SOCKET_SIZE	              32768  //TODO user defined socket_size
#define MESSAGE_SIZE	             1024
#define PATH_MAX	                  255
#define DOWNLOAD_PROGRESS_RESET    1000

char * getIp(char * domain);

int compile_regex(regex_t * r, const char * regex_text);

int match_regex (regex_t * r, const char * to_match);

int logError(char * msg);

void printDownloadProgress(int * dots);

#endif /* __UTILS_H */
