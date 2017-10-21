#ifndef LINK_LAYER_H
#define LINK_LAYER_H

#include "utils.h"

int initLinkLayer(int porta, int baudRate, uint timeout, uint numTransmissions);

int openSerialPort();

int llopen(ConnectionType type);

int llclose(int fd);

int llwrite(int fd, uchar * buffer, int length);

int llread(int fd, uchar ** buffer);

#endif
