#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define MAX_FILE_NAME	256

#define FALSE	0
#define TRUE	1

#define ERROR2 -1
#define ERROR	0
#define OK		1

extern int DEBUG;

typedef unsigned int uint;
typedef unsigned char uchar;

typedef enum {
  TRANSMITTER, RECEIVER
} ConnectionType ;

long getFileSize(FILE* file);

void printArray(uchar buffer[], int size);

void convertIntToBytes(uchar * res, uint src);	

uint convertBytesToInt(uchar * src);			

int logError(char * errorMsg);

int getBaudrate(int baudrate);

void printProgressBar(int current, int total);


#endif
