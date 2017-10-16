#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include "utils.h"
#include "Packets.h"

typedef struct {
    int fileDescriptor; // serial port's file descriptor
    char fileName[MAX_FILE_NAME];
    ConnectionType type;
} ApplicationLayer;


// TODO extract data/control packets structures and constructor functions to another file, OOP?
int initApplicationLayer(const char * port, ConnectionType type, int baudrate,
    int maxDataMsgSize, int numRetries, int timeout, char* file);

int sendFile();
int receiveFile();

// Makes control packet and sends it through low-level layer
int sendControlPacket(ControlPacket * src);

// Receives control packet through low-level layer and stores it in dest
int receiveControlPacket(ControlPacket * dest);

/**
 * Makes the DataPacket into a Packet and sends it through the LinkLayer (llwrite)
 * returns whether an error occurred
 */
int sendDataPacket(DataPacket * src);

/**
 * Receives a Packet through the LinkLayer and extracts the DataPacket
 * to the provided destinationwrite).
 * returns whether an error occurred
 */
int receiveDataPacket(DataPacket * dest);


#endif
