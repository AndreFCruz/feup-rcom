#include <stdlib.h>
#include <string.h>
#include "ApplicationLayer.h"
#include "utils.h"

#define CTRL_PACKET_ARGS	2
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

	ControlPacket ctrlPacket;
	ctrlPacket.argNr = CTRL_PACKET_ARGS;
	ctrlPacket.type = START;
	ctrlPacket.fileSize = getFileSize(file);

	if (sendControlPacket(&ctrlPacket) != OK) {
		printf("Error sending control packet.\n");
		return ERROR;
	}

	DataPacket dataPacket;
	unsigned char fileBuffer[FILE_BUFFER_SIZE];
	uint res, progress = 0, i = 0;
	while ( (res = fread(fileBuffer, sizeof(char), FILE_BUFFER_SIZE, file)) > 0 ) {
		dataPacket.seqNr = i++;
		dataPacket.packetSize = res;
		dataPacket.data = fileBuffer;
		if (!sendDataPacket(&dataPacket)) {
			printf("Error sending data packet.\n");
			return ERROR;
		}

		progress += res;
	}

	//free(fileBuffer);
	if (fclose(file) != OK) {
		perror("Error while closing file");
		return ERROR;
	}

	ctrlPacket.type = END;
	if (sendControlPacket(&ctrlPacket) != OK) {
		printf("Error sending control packet.\n");
		return ERROR;
	}

	if (llclose(al->fd) != OK)
		return ERROR;

	return OK;
}

int receiveFile() {
  return 0;
}
