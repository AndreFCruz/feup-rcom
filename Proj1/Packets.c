#include "Packets.h"
#include "LinkLayer.h"

void makeDataPacket(DataPacket * src, Packet * dest){
	int packetSize = HEADER_SIZE + (src->size);
	unsigned char * data = (unsigned char *) malloc(packetSize);

	data[CTRL_FIELD_IDX] = DATA;
	data[SEQ_NUM_IDX] = src->seqNr;
	data[DATA_PACKET_SIZE2_IDX] = (unsigned char) (src->size / SIZE2_MUL);
	data[DATA_PACKET_SIZE1_IDX] = (unsigned char) (src->size % SIZE2_MUL);

	memcpy(&data[HEADER_SIZE], src->data, src->size);

	dest->data = data;
	dest->size = packetSize;
}

void makeControlPacket(ControlPacket * src, Packet * dest){
	//printf("Filename: %s\n", src->fileName);
	int fileNameSize = strnlen(src->fileName, MAX_FILE_NAME);
	int packetSize = 1 + 2 * (src->argNr) + fileNameSize + FILE_SIZE_LENGTH;
	//printf("packetSize: %d, fileNameSize: %d\n", packetSize, (unsigned char) fileNameSize);

	unsigned char * data = (unsigned char *) malloc(packetSize);

	data[CTRL_FIELD_IDX] = src->type;

	int index = 1;
	data[index++] = FILE_SIZE_ARG;
	data[index++] = sizeof(int);

	unsigned char fileSize[sizeof(int)]; // TODO delete convertIntToBytes
	convertIntToBytes(fileSize, src->fileSize);
	memcpy(&data[index], fileSize, FILE_SIZE_LENGTH);

	printf("Size: %d\n", src->fileSize);
	//((uint *) (data + index)) = src->fileSize;
	//memcpy(&data[index], (uchar*) &src->fileSize, sizeof(int));

	index += FILE_SIZE_LENGTH;

	data[index++] = FILE_NAME_ARG;
	data[index++] = (unsigned char) fileNameSize;
	memcpy(&data[index], src->fileName, fileNameSize);

	dest->data = data;
	dest->size = packetSize;
}

// TODO funcao generica sendPacket que recebe a funcao makePacket (of correct type)
int sendDataPacket(int fd, DataPacket * src) {
	Packet packet;
	makeDataPacket(src, &packet);
	printArray(packet.data, packet.size);
	int written = llwrite(fd, packet.data, packet.size);
	free(packet.data);
	if (written >= (int) packet.size)
		return OK;	//TODO METER BONITINHO

	printf("Sent DATA packet with size %d/%d\n", written, (int) packet.size);
	return ERROR;
}

int sendControlPacket(int fd, ControlPacket * src){
	Packet packet;
	makeControlPacket(src, &packet);
	printf("Sending control packet: "); printArray(packet.data, packet.size);
	int written = llwrite(fd, packet.data, packet.size);
	free(packet.data);
	if (written >= (int) packet.size)
		return OK;

	printf("Sent CONTROL packet with size %d/%d\n", written, (int) packet.size);
	return ERROR;
}

int receiveDataPacket(int fd, DataPacket * dest) {
	char * data;
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

		/*
		uint size = *(uint*) (data+1);
		printf("Val: %0X\n", size);
		memcpy(&size, data+4, sizeof(int));			//ESTAS FORMAS TROCAM OS BYTES - FICA O REVERSO DO ORIGINAL
		printf("Val: %0X\n", size);
		*/

		dest->fileSize = size;
	}
	else if(argNr == 1){
		//strcpy(dest->fileName, (char *) (data+offset));  //TODO: VER PORQUE NAO RESULTA
		memcpy(dest->fileName, (data+offset), argSize);
	}
	else
		return ERROR;

	return OK;
}

int receiveControlPacket(int fd, ControlPacket * dest) {
	char * data;
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
