#include <stdlib.h>
#include <string.h>
#include "ApplicationLayer.h"
#include "utils.h"

#define CTRL_PACKET_ARGS	2
#define FILE_BUFFER_SIZE	256

static ApplicationLayer * al;

int sendFile() {
	if (al == NULL)
		return logError("AL not initialized");

	FILE * file = fopen(al->fileName, "r"); // may need to be changed to "ab" for compatibility
	if (file == NULL)
		return logError("Error while opening file");

	// establish connection
	al->fd = llopen(al->type);
	if (al->fd < 0)
		return ERROR;

	ControlPacket ctrlPacket;
	ctrlPacket.argNr = CTRL_PACKET_ARGS;
	ctrlPacket.type = START;
	ctrlPacket.fileSize = getFileSize(file);

	if (sendControlPacket(al->filelDescriptor, &ctrlPacket) != OK)
		return logError("Error sending control packet");

	DataPacket dataPacket;
	unsigned char fileBuffer[FILE_BUFFER_SIZE];
	uint res, progress = 0, i = 0;
	while ( (res = fread(fileBuffer, sizeof(char), FILE_BUFFER_SIZE, file)) > 0 ) {
		dataPacket.seqNr = i++;
		dataPacket.packetSize = res;
		dataPacket.data = fileBuffer;
		if (!sendDataPacket(al->filelDescriptor, &dataPacket))
			return logError("Error sending data packet");

		progress += res;
	}

	if (fclose(file) != OK) {
		perror("Error while closing file");
		return ERROR;
	}

	ctrlPacket.type = END;
	if (sendControlPacket(al->filelDescriptor, &ctrlPacket) != OK)
		return logError("Error sending control packet");

	if (llclose(al->fd) != OK)
		return ERROR;

	return OK;
}

int receiveFile() {
	if (al == NULL)
		return logError("AL not initialized");

	al->fd = llopen(al->type);
	if (fd <= 0)
		return 0;

	ControlPacket ctrlPacket;
	if (receiveControlPacket(&ctrlPacket) != OK || ctrlPacket->type != START) {
		return logError("Error receiving control packet");
	}

	al->fileName = ctrlPacket.fileName;

	FILE * outputFile = fopen(al->fileName, "wb");
	if (outputFile == NULL)
		return logError("Could not create output file");

	// TODO create function to print **pretty** messages :)
	printf("Created file %s with expected size %d.\n", al->fileName, ctrlPacket.fileSize);

	uint res, progress = 0;
	while (progress < ctrlPacket.fileSize) {
		
	}

  return 0;
}
