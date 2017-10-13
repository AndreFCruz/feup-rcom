#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include "utils.h"

#define MAX_FILE_NAME 255

typedef struct {
  int fileDescriptor; // serial port's file descriptor
  ConnectionType status;
} ApplicationLayer;

typedef struct {
  unsigned char * data;
  unsigned int size;
} Packet;

typedef struct {
  unsigned char seqNr;
  unsigned int packetSize;
  unsigned char * data;
} DataPacket;

typedef struct {
	int fileSize;
	char fileName[MAX_FILE_NAME];
	int argNr;
} ControlPacket;

int sendFile();
int receiveFile();

int sendControlPacket(ControlPacket * src);
int receiveControlPacket(ControlPacket * dest);

Packet * sendDataPacket(DataPacket * packet);
Packet * receiveDataPacket(DataPacket * packet);


#endif
