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

	if (initLinkLayer(atoi(port), baudrate, timeout, numRetries) != OK)
		return logError("Failed LL initialization");

	if ((al->fd = openSerialPort()) == -1)
		return logError("Failed open serial port");

	strncpy(al->fileName, file, MAX_FILE_NAME);
	//printf("Filename: %s\n", al->fileName);
	al->type = type;
	al->maxDataMsgSize = maxDataMsgSize;

	return OK;
}

int sendFile() {
	if (al == NULL)
		return logError("AL not initialized");

	FILE * file = fopen(al->fileName, "r"); // may need to be changed to "ab" for compatibility
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

	printf("Sending control packet...\n");

	if (sendControlPacket(al->fd, &ctrlPacket) != OK)
		return logError("Error sending control packet");

	DataPacket dataPacket;
	uchar * fileBuffer = (uchar *) malloc(al->maxDataMsgSize * sizeof(char));
	uint res, progress = 0, i = 0;
	while ( (res = fread(fileBuffer, sizeof(char), al->maxDataMsgSize, file)) > 0 ) {
		dataPacket.seqNr = i++;
		dataPacket.size = res;
		dataPacket.data = fileBuffer;
		if (sendDataPacket(al->fd, &dataPacket) != OK) {
			logError("Error sending data packet");
			break;
		}

		progress += res;
	}
	free(fileBuffer);

	if (fclose(file)) {
		perror("Error while closing file");
		return ERROR;
	}

	ctrlPacket.type = END;
	if (sendControlPacket(al->fd, &ctrlPacket) != OK)
		return logError("Error sending control packet");

	if (llclose(al->fd) != OK)
		return ERROR;

	return OK;
}

int receiveFile() {
	if (al == NULL)
		return logError("AL not initialized");

	al->fd = llopen(al->type);
	if (al->fd < 0)
		return logError("Failed llopen");

	printf("Receiving control packet...\n");

	ControlPacket ctrlPacket;
	if (receiveControlPacket(al->fd, &ctrlPacket) != OK || ctrlPacket.type != START) {
		return logError("Error receiving control packet");
	}

	strncpy(al->fileName, ctrlPacket.fileName, MAX_FILE_NAME);

	FILE * outputFile = fopen(al->fileName, "wb");
	if (outputFile == NULL)
		return logError("Could not create output file");

	// TODO create function to print **pretty** messages :)
	printf("Created file %s with expected size %d.\n", al->fileName, ctrlPacket.fileSize);

	DataPacket dataPacket;
	uint res, progress = 0, totalPackets = 0;
	while (progress < ctrlPacket.fileSize) {
		if (receiveDataPacket(al->fd, &dataPacket) != OK) {
			logError("Error sending data packet");
			break;
		}
		progress += (uint) dataPacket.size;

		printf("PROGRESS: %d, FILESIZE: %d, DATAPACKETSIZE: 0x%02X\n", progress, ctrlPacket.fileSize, dataPacket.size);

		if (fwrite(dataPacket.data, sizeof(char), dataPacket.size, outputFile) == 0) {
			printf("fwrite returned 0\n");
			return OK;
		}

	}
	printf("OUT OF DATA PACKETS \n");

	if (fclose(outputFile)) {
		perror("fclose failed");
		return ERROR;
	}

	if (receiveControlPacket(al->fd, &ctrlPacket) != OK || ctrlPacket.type != END) {
		return logError("Error receiving control packet");
	}

	if (!llclose(al->fd))
		return logError("llclose failed");

	printf("File received successfully.\n");

	return OK;
}
