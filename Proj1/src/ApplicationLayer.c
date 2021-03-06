#include <stdlib.h>
#include <string.h>
#include "ApplicationLayer.h"
#include "LinkLayer.h"
#include "utils.h"

#define CTRL_PACKET_ARGS	2
#define FILE_BUFFER_SIZE	256

static ApplicationLayer * al = NULL;

int initApplicationLayer(const char * port, int baudrate, int timeout, int numRetries, ConnectionType type, int maxDataMsgSize, char * file) {
	if (al == NULL)
		al = malloc(sizeof(ApplicationLayer));
	else
		return logError("ApplicationLayer already initialized");

	if (initLinkLayer(atoi(port), getBaudrate(baudrate), timeout, numRetries) != OK)
		return logError("Failed LL initialization");

	if ((al->fd = openSerialPort()) == -1)
		return logError("Failed open serial port");

  if (file == NULL)
		al->fileName = NULL;
	else {
		al->fileName = (char *) malloc(sizeof(char) * MAX_FILE_NAME);
		strncpy(al->fileName, file, MAX_FILE_NAME);
	}

	al->type = type;
	al->maxDataMsgSize = maxDataMsgSize;

	return OK;
}

void destroyApplicationLayer() {
	if (al->fileName != NULL)
		free(al->fileName);
	free(al);

	al = NULL;
}


int sendFile() {
	if (al == NULL)
		return logError("AL not initialized");

	FILE * file = fopen(al->fileName, "r");
	if (file == NULL)
		return logError("Error while opening file");

	// establish connection
	al->fd = llopen(al->type);
	if (al->fd < 0)
		return logError("Failed llopen");

	ControlPacket ctrlPacket;
	ctrlPacket.type = START;
	strcpy(ctrlPacket.fileName, al->fileName);
	ctrlPacket.fileSize = getFileSize(file);
	ctrlPacket.argNr = CTRL_PACKET_ARGS;

	if (sendControlPacket(al->fd, &ctrlPacket) != OK)
		return logError("Error sending control packet");

	DataPacket dataPacket;
	uchar * fileBuffer = (uchar *) malloc(al->maxDataMsgSize * sizeof(char));
	uint res, progress = 0, currentSeqNr = 0;
	int state = OK;
	printProgressBar(0, ctrlPacket.fileSize);
	
	while ( (res = fread(fileBuffer, sizeof(char), al->maxDataMsgSize, file)) > 0 ) {
		dataPacket.seqNr = currentSeqNr;
		currentSeqNr = (currentSeqNr + 1) % 256;
		dataPacket.size = res;
		dataPacket.data = fileBuffer;
		if (sendDataPacket(al->fd, &dataPacket) != OK) {
			state = logError("Error sending data packet");
			break;
		}

		progress += res;
		printProgressBar(progress, ctrlPacket.fileSize);
	}
	free(fileBuffer);

	if (fclose(file)) {
		perror("Error while closing file");
		return ERROR;
	}

	ctrlPacket.type = END;
	if ((state == OK) && sendControlPacket(al->fd, &ctrlPacket) != OK)
		return logError("Error sending control packet");

	if (state != OK || llclose(al->fd) != OK)
		return ERROR;

	printf("\nFile sent successfully.\n");

	return OK;
}


int receiveFile() {
	if (al == NULL)
		return logError("AL not initialized");

	al->fd = llopen(al->type);
	if (al->fd < 0)
		return logError("Failed llopen");

	ControlPacket ctrlPacket;
	if (receiveControlPacket(al->fd, &ctrlPacket) != OK || ctrlPacket.type != START) {
		return logError("Error receiving control packet");
	}

	if (al->fileName == NULL) {
		al->fileName = malloc(sizeof(char) * MAX_FILE_NAME);
		strncpy(al->fileName, ctrlPacket.fileName, MAX_FILE_NAME);
	}

	FILE * outputFile = fopen(al->fileName, "wb");
	if (outputFile == NULL)
		return logError("Could not create output file");

	printf("Created file %s with expected size %d.\n", al->fileName, ctrlPacket.fileSize);

	DataPacket dataPacket;
	uint progress = 0, currentSeqNr = 0;
	int state = OK;
	printProgressBar(0, ctrlPacket.fileSize);

	while (progress < ctrlPacket.fileSize) {
		if ( (state = receiveDataPacket(al->fd, &dataPacket)) != OK) {
			logError("Error receiving data packet");
			break;
		}

		if (dataPacket.seqNr < currentSeqNr) {
			printf("\tReceived duplicate packet: %d. Current: %d\n", dataPacket.seqNr, currentSeqNr);
			continue;
		}
		currentSeqNr = (currentSeqNr + 1) % 256;
		progress += (uint) dataPacket.size;

		printProgressBar(progress, ctrlPacket.fileSize);

		if (fwrite(dataPacket.data, sizeof(char), dataPacket.size, outputFile) == 0) {
			return logError("sendFile: fwrite returned 0");
		}

		free(dataPacket.data);
	}

	if (fclose(outputFile)) {
		perror("fclose failed");
		return ERROR;
	}

	if ((state == OK) && (receiveControlPacket(al->fd, &ctrlPacket) != OK || ctrlPacket.type != END)) {
		return logError("Error receiving control packet");
	}

	if (state != OK || llclose(al->fd) != OK)
		return logError("llclose failed");

	printf("\nFile received successfully.\n");

	return OK;
}