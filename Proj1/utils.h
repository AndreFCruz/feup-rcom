#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define ERROR	1
#define OK		0

typedef unsigned int uint;
typedef unsigned char uchar;

enum ConnectionType {
  TRANSMITTER, RECEIVER
};

uint getFileSize(FILE * file);

#endif
