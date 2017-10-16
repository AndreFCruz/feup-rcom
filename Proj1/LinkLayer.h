#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include "utils.h"

int initLinkLayer(char port[], int baudRate, uint timeout, uint numTransmissions);

int openSerialPort();

int llopen(int porta, ConnectionType type);

int llclose(int fd, ConnectionType type);

int llwrite(int fd, char * buffer, int length);

int llread(int fd, char ** buffer);

#endif
