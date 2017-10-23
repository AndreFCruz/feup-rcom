#include "Packets.h"
#include "LinkLayer.h"

void makeDataPacket(DataPacket * src, Packet * dest) {
	int packetSize = HEADER_SIZE + (src->size);
	uchar * data = (uchar *) malloc(packetSize);

	data[CTRL_FIELD_IDX] = DATA;
	data[SEQ_NUM_IDX] = src->seqNr;
	data[DATA_PACKET_SIZE2_IDX] = (uchar) (src->size / SIZE2_MUL);
	data[DATA_PACKET_SIZE1_IDX] = (uchar) (src->size % SIZE2_MUL);

	memcpy(&data[HEADER_SIZE], src->data, src->size);

	dest->data = data;
	dest->size = packetSize;
}

void makeControlPacket(ControlPacket * src, Packet * dest) {
	printf(" ... making control packet ... \n");

	int fileNameSize = strnlen(src->fileName, MAX_FILE_NAME);
	int packetSize = 1 + 2 * (src->argNr) + fileNameSize + FILE_SIZE_LENGTH;

	uchar * data = (uchar *) malloc(packetSize);

	data[CTRL_FIELD_IDX] = src->type;

	int index = 1;
	data[index++] = FILE_SIZE_ARG;
	data[index++] = sizeof(int);

	uchar fileSize[sizeof(int)];
	convertIntToBytes(fileSize, src->fileSize);
	memcpy(&data[index], fileSize, FILE_SIZE_LENGTH);

	printf("Size: %d\n", src->fileSize);
	//((uint *) (data + index)) = src->fileSize;
	//memcpy(&data[index], (uchar*) &src->fileSize, sizeof(int));

	index += FILE_SIZE_LENGTH;

	data[index++] = FILE_NAME_ARG;
	data[index++] = (uchar) fileNameSize;
	memcpy(&data[index], src->fileName, fileNameSize);

	dest->data = data;
	dest->size = packetSize;
}

int sendDataPacket(int fd, DataPacket * src) {
	Packet packet;
	makeDataPacket(src, &packet);
	int written = llwrite(fd, &(packet.data), packet.size);
	free(packet.data);
	if (written >= (int) packet.size)
		return OK;

	printf("Sent DATA packet with size %d/%d\n", written, (int) packet.size);
	return ERROR;
}

int sendControlPacket(int fd, ControlPacket * src) {
	Packet packet;
	makeControlPacket(src, &packet);
	printf("Sending control packet: "); printArray(packet.data, packet.size);
	int written = llwrite(fd, &(packet.data), packet.size);
	free(packet.data);
	if (written >= (int) packet.size)
		return OK;

	printf("Sent CONTROL packet with size %d/%d\n", written, (int) packet.size);
	return ERROR;
}

int receiveDataPacket(int fd, DataPacket * dest) {
	uchar * data;
	if(llread(fd, &data) <= 0)
		return logError("failed to read packet");

	if(data[CTRL_FIELD_IDX] != DATA) {
		printf("type does not match any known type DATA, was %02X\n", data[CTRL_FIELD_IDX]);
		return ERROR;
	}

	dest->seqNr = data[SEQ_NUM_IDX];
	dest->size = (uchar) data[DATA_PACKET_SIZE2_IDX] * SIZE2_MUL + (uchar) data[DATA_PACKET_SIZE1_IDX];
	printf("Received data packet size: %d\n", (int) dest->size);
	dest->data = (uchar *) malloc(dest->size);
	memcpy(dest->data, &data[HEADER_SIZE], dest->size);
	return OK;
}

int fillControlPacketArg(uchar * data, ControlPacket * dest, int argNr, int argSize, int offset) {
	if(argNr == 0){
		uint size = convertBytesToInt(data+offset);
		dest->fileSize = size;
	}
	else if(argNr == 1){
		memcpy(dest->fileName, (data+offset), argSize);
	}
	else
		return ERROR;

	return OK;
}

int receiveControlPacket(int fd, ControlPacket * dest) {
	uchar * data;
	int dataSize;
	if((dataSize = llread(fd, &data)) < 0)
		return logError("failed to read packet");

	printf("llread succeeded\ndataSize: %d\n", dataSize);

	printArray(data, dataSize);

	dest->type = data[CTRL_FIELD_IDX];

	if(dest->type != START && dest->type != END)
		return logError("type does not match any known type (START, END)");

	int index = 1, argNr = 0, argSize;
	while(index < dataSize){
		if(data[index++] != argNr)
			return logError("wrong sequence of arguments");

		argSize = data[index++];
		if(!fillControlPacketArg(data, dest, argNr, argSize, index))
			return logError("could not fill argument");
		index += argSize;
		argNr++;
	}

	dest->argNr = argNr;

	//printf("Size: %d, Name: %s\n",dest->fileSize,dest->fileName);
	return OK;
}
