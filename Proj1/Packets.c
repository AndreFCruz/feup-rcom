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

int sendDataPacket(int fd, DataPacket * src) {
	Packet packet;
	makeDataPacket(src, &packet);
	int written = llwrite(fd, &(packet.data), packet.size);
	//free(packet.data); // TODO
	if (written >= (int) packet.size)
		return OK;

	return ERROR;
}

void makeControlPacket(ControlPacket * src, Packet * dest) {
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

	index += FILE_SIZE_LENGTH;

	data[index++] = FILE_NAME_ARG;
	data[index++] = (uchar) fileNameSize;
	memcpy(&data[index], src->fileName, fileNameSize);

	dest->data = data;
	dest->size = packetSize;
}

int sendControlPacket(int fd, ControlPacket * src) {
	Packet packet;
	makeControlPacket(src, &packet);
	int written = llwrite(fd, &(packet.data), packet.size);
	free(packet.data);
	if (written >= (int) packet.size)
		return OK;
	return ERROR;
}

int receiveDataPacket(int fd, DataPacket * dest) {
	uchar * buffer;
	if(llread(fd, &buffer) <= 0)
		return logError("receiveDataPacket: failed to read packet");

	if(buffer[CTRL_FIELD_IDX] != DATA) {
		return logError("receiveDataPacket: type does not match any known type DATA");
	}

	dest->seqNr = buffer[SEQ_NUM_IDX];
	dest->size = (uchar) buffer[DATA_PACKET_SIZE2_IDX] * SIZE2_MUL + (uchar) buffer[DATA_PACKET_SIZE1_IDX];
	dest->data = (uchar *) malloc(dest->size);
	memcpy(dest->data, &buffer[HEADER_SIZE], dest->size);
	free(buffer);
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
		return logError("receiveControlPacket: failed to read packet");

	dest->type = data[CTRL_FIELD_IDX];

	if(dest->type != START && dest->type != END)
		return logError("receiveControlPacket: type does not match any known type (START, END)");

	int index = 1, argNr = 0, argSize;
	while(index < dataSize){
		if(data[index++] != argNr)
			return logError("receiveControlPacket: wrong sequence of arguments");

		argSize = data[index++];
		if(!fillControlPacketArg(data, dest, argNr, argSize, index))
			return logError("receiveControlPacket: could not fill argument");
		index += argSize;
		argNr++;
	}

	dest->argNr = argNr;

	return OK;
}
