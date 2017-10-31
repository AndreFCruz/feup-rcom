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

#define BAUDRATE 38400
#define _IDXIX_SOURCE 1 /* POSIX compliant source */
#define NUM_RETRIES 3
#define TIMEOUT 	3
#define DATA_BYTES	32
#define MAX_DATA_BYTES	(256*256 - 1)


void printUsage(char * progName) {
	printf("Usage:\t%s <SerialPort> <r/w> <FILE_NAME> [BAUDRATE] [DATA_BYTES] [NUM_RETRIES] [TIMEOUT]\n", progName);
	printf("\tex: %s 0 w pinguim.gif\n", progName);
	printf("Arguments between '[' ']' are optional\n");
}

void printSettings(const char * port, int baudrate, int timeout, int numRetries, ConnectionType type, int dataBytes, const char * fileName) {
	printf("\n\t** Settings: **\n");
	printf("\tType: %16s\n", type == TRANSMITTER ? "TRANSMITTER" : "RECEIVER");
	printf("\tFile name: %10s\n", fileName == NULL ? " ** not set ** " : fileName);
	printf("\tNumber of retries: %d\n", numRetries);
	printf("\tTimeout (in seconds): %d\n", timeout);
	printf("\tData bytes: %4d\n", dataBytes);
	printf("\tBaud rate: %5d\n", baudrate);
	printf("\tPort used: %5s\n", port);
	printf("\t\t* * *\n\n");
}


int main(int argc, char** argv)
{
	DEBUG = TRUE;

	if (argc < 2 || ((strcmp("0", argv[1])!=0) && (strcmp("1", argv[1])!=0))) {
		printUsage(argv[0]);
		exit(1);
	}

	ConnectionType type;
	int (*functionPtr)(void);

	if ( strcmp("w", argv[2]) == 0 && argc > 3 ) {
		type = TRANSMITTER;
		functionPtr = &sendFile;
	} else if ( strcmp("r", argv[2]) == 0 && argc > 2 ) {
		type = RECEIVER;
		functionPtr = &receiveFile;
	} else {
		printUsage(argv[0]);
		exit(1);
	}

	char * fileName = argc > 3 ? argv[3] : NULL;

	int baudRate = BAUDRATE;
	if (argc > 4)
		baudRate = strtol(argv[4], NULL, 10);

	int dataBytes = DATA_BYTES;
	if (argc > 5)
		dataBytes = strtol(argv[5], NULL, 10);
	if (dataBytes > MAX_DATA_BYTES)
		dataBytes = MAX_DATA_BYTES;

	int numRetries = NUM_RETRIES;
	if (argc > 6)
		numRetries = strtol(argv[6], NULL, 10);

	int timeout = TIMEOUT;
	if (argc > 7)
		timeout = strtol(argv[7], NULL, 10);


	printSettings(argv[1], baudRate, timeout, numRetries, type, dataBytes, fileName);

	if (initApplicationLayer(argv[1], baudRate, timeout, numRetries, type, dataBytes, fileName) == ERROR)
		return logError("main.c: failed ApplicationLayer initialization");

	(*functionPtr)();

	destroyApplicationLayer();

	return 0;
}
