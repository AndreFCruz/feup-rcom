#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include "utils.h"

#define MAX_PORT_NAME 32

typedef unsigned int uint;

typedef struct {
	char port[MAX_PORT_NAME];
	int baudRate;
	uint sequenceNumber;
	uint timeout;
	uint numTransmissions;
} LinkLayer;

int initLinkLayer(char port[], int baudRate, uint timeout, uint numTransmissions, char frame[]);

int openSerialPort(LinkLayer * ptr);

int llopen(int porta, ConnectionType type);

int llclose(int fd);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char * buffer);


#endif
