#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ApplicationLayer.h"

#define CTRL_PACKET_ARGS    2
#define FILE_BUFFER_SIZE	256

ApplicationLayer * al;


int sendFile() {
	FILE * file = fopen(al->fileName, "r"); // may need to be changed to "ab" for compatibility
	if (file == NULL) {
		printf("Error while opening file.\n");
		return ERROR;
	}

	// establish connection
	al->fd = llopen(al->type);
	if (al->fd < 0)
		return ERROR;

	ControlPacket packet;
	packet.argNr = CTRL_PACKET_ARGS;
	// packet.fileSize

	DataPacket data;
	if ((data = sendControlPacket(packet)) == NULL)
		return ERROR;

	char fileBuffer[FILE_BUFFER_SIZE];
	uint res, writtenBytes = 0;
	while ( (res = fread(fileBuffer, sizeof(char), FILE_BUFFER_SIZE, file)) > 0 ) {

	}

	return 0;
}

int receiveFile() {
  return 0;
}
