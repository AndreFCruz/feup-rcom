#ifndef __UTILS_H
#define __UTILS_H

#include <sys/types.h>
#include <regex.h>

// #define BOOL	int
#define FALSE	0
#define TRUE	1

#define SOCKET_SIZE	32768  //TODO user defined socket_size
#define MESSAGE_SIZE	1024

char * getIp(char * domain);
int compile_regex(regex_t * r, const char * regex_text);
int match_regex (regex_t * r, const char * to_match);
int logError(char * msg);

#endif /* __UTILS_H */
