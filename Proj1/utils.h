#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define ERROR	1
#define OK		0

typedef unsigned int uint;
typedef unsigned char uchar;

typedef enum {
  TRANSMITTER, RECEIVER
} ConnectionType ;

void printArray(uchar buffer[], int size);
void convertIntToBytes(uchar * res, uint src);
uint convertBytesToInt(uchar * src);
uint getFileSize(FILE * file);

#endif
