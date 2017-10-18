#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ApplicationLayer.h"
#include "utils.h"

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS0" //TODO: delete
#define _IDXIX_SOURCE 1 /* POSIX compliant source */
#define NUM_RETRIES 3
#define TIMEOUT 	3
#define FILE_NAME "pinguim.gif"
#define MAX_DATA	10

int DEBUG = FALSE; // TODO

void printUsage(char * progName) {
	printf("Usage:\t%s SerialPort r/w\n\tex: %s 0 w\n", progName, progName);
}

int main(int argc, char** argv)
{
	if ( (argc < 3) ||
		((strcmp("0", argv[1])!=0) && (strcmp("1", argv[1])!=0)) ) {
		printUsage(argv[0]);
		exit(1);
	}

	ConnectionType type;
	int (*functionPtr)(void);

	if ( strcmp("w", argv[2]) == 0 ) {
		type = TRANSMITTER;
		functionPtr = &sendFile;
	} else if ( strcmp("r", argv[2]) == 0 ) {
		type = RECEIVER;
		functionPtr = &receiveFile;
	} else {
		printUsage(argv[0]);
		exit(1);
	}

	initApplicationLayer(argv[1], BAUDRATE, TIMEOUT, NUM_RETRIES, type, MAX_DATA, FILE_NAME);

	(*functionPtr)();

	return 0;
}
