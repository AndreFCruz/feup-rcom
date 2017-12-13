#ifdef  __UTILS_H
#define __UTILS_H

<<<<<<< HEAD
#define BOOL	int
#define FALSE	0
#define TRUE	1

#define SOCKET_SIZE	32768
#define MESSAGE_SIZE	1024
=======
#include <regex.h>
>>>>>>> 4da629d4eb5b9ad9fc1bfcdba34717bdb39077c4

char * getIp(char * domain);
int compile_regex(regex_t * r, const char * regex_text);
int match_regex (regex_t * r, const char * to_match);

#endif /* __UTILS_H */
