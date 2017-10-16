#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include "utils.h"
#include "Packets.h"

typedef struct {
    int fd; // serial port's file descriptor
    char fileName[MAX_FILE_NAME];
    ConnectionType type;
} ApplicationLayer;


// TODO extract data/control packets structures and constructor functions to another file, OOP?
int initApplicationLayer(const char * port, ConnectionType type, int baudrate,
    int maxDataMsgSize, int numRetries, int timeout, char* file);

int sendFile();
int receiveFile();


#endif
