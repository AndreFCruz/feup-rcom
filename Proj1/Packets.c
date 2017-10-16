#include "Packets.h"

void makeControlPacket(ControlPacket * src, Packet * dest){
	int packetSize = HEADER_SIZE + (src->size);
	unsigned char * data = (unsigned char *) malloc(packetSize);

	data[CTRL_FIELD_IDX] = DATA;
	data[SEQ_NUM_IDX] = src->seqNr;
	data[DATA_PACKET_SIZE2_IDX] = (unsigned char) (src->size / SIZE2_MUL);
	data[DATA_PACKET_SIZE1_IDX] = (unsigned char) (src->size % SIZE2_MUL);

	memcpy(&data[HEADER_SIZE], src->data, src->size);

	packet->data = data;
	packet->size = packetSize;
}

void makeDataPacket(DataPacket * src, Packet * dest){
	int fileNameSize = strnlen(al->fileName, MAX_FILE_NAME);
	int packetSize = 1 + 2 * (src->argNr) + fileNameSize + FILE_SIZE_LENGTH;
	//printf("packetSize: %d, fileNameSize: %d\n", packetSize, (unsigned char) fileNameSize);

	unsigned char * data = (unsigned char *) malloc(packetSize);

	data[CTRL_FIELD_IDX] = START;

	int index = 1;
	data[index++] = FILE_SIZE_ARG;
	data[index++] = sizeof(int);
	((uint *) (data + index)) = src->fileSize;
	
	unsigned char fileSize[sizeof(int)]; // TODO delete convertIntToBytes
	convertIntToBytes(fileSize, src->fileSize);
	memcpy(&data[index], fileSize, FILE_SIZE_LENGTH);

	printf("Size: %d\n", src->fileSize);
	//memcpy(&data[index], (uchar*) &src->fileSize, sizeof(int));

	index += FILE_SIZE_LENGTH;

	data[index++] = FILE_NAME_IDX;
	data[index++] = (unsigned char) fileNameSize;
	memcpy(&data[index], al->fileName, fileNameSize);

	packet->data = data;
	packet->size = packetSize;
}


int sendDataPacket(DataPacket * src) {
	Packet packet;
	makeDataPacket(src, &packet);
	int status = llwrite(al->fileDescriptor, packet.data, packet.size);
	free(packet.data);
	return status;
}

int sendControlPacket(ControlPacket * src){
	Packet packet;
	makeControlPacket(src, &packet);
	int status = llwrite(al->fileDescriptor, packet.data, packet.size);
	free(packet.data);
	return status;
}

int receiveDataPacket(DataPacket * dest) {
	Packet packet;
	if(llread(al->fileDescriptor, packet.data, packet.size))
		return logError("failed to read packet");

	uchar * data = p->data;
	if(data[CTRL_FIELD_IDX] != DATA)
		return logError("type does not match any known type (START, END)");
	
	dest->seqNr = data[SEQ_NUM_IDX];
	int size = data[DATA_PACKET_SIZE2_IDX] * SIZE + data[DATA_PACKET_SIZE1_IDX];
	dest->data = (uchar *) malloc(size);
	memcpy(dest->data, &data[HEADER_SIZE], size);
	free(dest->data);
	return OK;
}


// mudar nome da função para algo maix explicativo
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
		strcpy(dest->fileName, (char *) (data+offset));
	}
	else
		return ERROR;

	return OK;
}

int receiveControlPacket(ControlPacket * dest) {
	Packet packet;
	if(llread(al->fileDescriptor, packet.data, packet.size))
		return logError("failed to read packet");

	uchar * data = packet.data;
	dest->type = data[CTRL_FIELD_IDX];

	if(dest->type != START || dest->type != END)
		return logError("type does not match any known type (START, END)");

	int index = 1, argNr = 0, argSize;
	while(index < packet.size){
		if(data[index++] != argNr)
			return logError("wrong sequence of arguments");

		argSize = data[index++];
		if(fillControlPacketArg(data, dest, argNr, argSize, index))
			return logError("could not fill argument");
		index += argSize;
		argNr++;
	}

	dest->argNr = argNr;

	printf("Size: %d, Name: %s\n",dest->fileSize,dest->fileName);
	return OK;
}