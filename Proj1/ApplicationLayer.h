#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include "utils.h"
#include "Packets.h"

typedef struct {
    int fd; // serial port's file descriptor
    char * fileName;
    ConnectionType type;
    int maxDataMsgSize;
} ApplicationLayer;


int initApplicationLayer(const char * port, int baudrate, int timeout, int numRetries, ConnectionType type, int maxDataMsgSize, char * file);

void destroyApplicationLayer();

int sendFile();

int receiveFile();


#endif
