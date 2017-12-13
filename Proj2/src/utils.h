#ifndef __UTILS_H
#define __UTILS_H

#include <regex.h>

char * getIp(char * domain);
int compile_regex(regex_t * r, const char * regex_text);
int match_regex (regex_t * r, const char * to_match);

#endif /* __UTILS_H */