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
#define MODEMDEVICE "/dev/ttyS0"
#define _IDXIX_SOURCE 1 /* POSIX compliant source */

void printUsage();
void initSender();
void initReceiver();

int main(int argc, char** argv)
{
	if ( (argc < 3) || 
		((strcmp("0", argv[1])!=0) && (strcmp("1", argv[1])!=0)) ) {
		printUsage(argv[0]);
		exit(1);
	}

	ConnectionType type;

	if ( strcmp("w", argv[2]) == 0 ) {
		type = TRANSMITTER;
	} else if ( strcmp("r", argv[2]) != 0 ) {
		type = RECEIVER;
	} else {
		printUsage(argv[0]);
		exit(1);
	}

	initApplicationLayer(argv[1], type, BAUDRATE, , int numRetries, int timeout, char* file);

	return 0;
}


void printUsage(char * progName) {
	printf("Usage:\t%s SerialPort r/w\n\tex: %s 0 w\n", progName, progName);
}

void initSender() {
	
}

void initReceiver() {
	
}
