#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ApplicationLayer.h"

#define SIZE2_MUL			256
#define HEADER_SIZE_IDX		4


//DATA_PACKET
#define CTRL_FIELD_IDX		0
#define SEQ_NUM_IDX			1
#define DATA_PACKET_SIZE2_IDX	2
#define DATA_PACKET_SIZE1_IDX	3

//CONTROL_PACKET
#define FILE_SIZE_IDX		0
#define FILE_NAME_IDX		1

#define FILE_SIZE_LENGTH	4

ApplicationLayer * al;
Packet * p;

int sendDataPacket(DataPacket * src) {
	int packetSize = HEADER_SIZE + (src->size);
	unsigned char * data = (unsigned char *) malloc(packetSize);

	data[CTRL_FIELD_IDX] = DATA;
	data[SEQ_NUM_IDX] = src->seqNr;
	data[DATA_PACKET_SIZE2_IDX] = (unsigned char) (src->size / SIZE2_MUL);
	data[DATA_PACKET_SIZE1_IDX] = (unsigned char) (src->size % SIZE2_MUL);

	memcpy(&data[HEADER_SIZE], src->data, src->size);

	Packet packet;
	packet.data = data;
	packet.size = packetSize;

	printArray(packet.data, packet.size);
	p = &packet;

	//return llwrite(packet);
	return 0;
}

int receiveDataPacket(DataPacket * dest) {
	uchar * data = p->data;
	if(data[CTRL_FIELD_IDX] != 1)
		return ERROR;
	dest->seqNr = data[SEQ_NUM_IDX];
	int size = data[DATA_PACKET_SIZE2_IDX] * SIZE + data[DATA_PACKET_SIZE1_IDX];
	dest->data = (uchar *) malloc(size);
	memcpy(dest->data, &data[HEADER_SIZE], size);
	free(dest->data);
	return OK;
}

// dividir como estava antes, makeControlPacket e sendControlPacket chama-a
int sendControlPacket(ControlPacket * src){
	int fileNameSize = strnlen(al->fileName, MAX_FILE_NAME);
	int packetSize = 1 + 2 * (src->argNr) + fileNameSize + FILE_SIZE_LENGTH;
	//printf("packetSize: %d, fileNameSize: %d\n", packetSize, (unsigned char) fileNameSize);

	unsigned char * data = (unsigned char *) malloc(packetSize);

	int index = 1;
	data[CTRL_FIELD_IDX] = 2;

	data[index++] = FILE_SIZE_IDX;
	data[index++] = FILE_SIZE_LENGTH;
	unsigned char fileSize[sizeof(int)]; // TODO delete convertIntToBytes
	convertIntToBytes(fileSize, src->fileSize);
	memcpy(&data[index], fileSize, FILE_SIZE_LENGTH);

	printf("Size: %d\n", src->fileSize);
	//memcpy(&data[index], (uchar*) &src->fileSize, sizeof(int));

	index += FILE_SIZE_LENGTH;

	data[index++] = FILE_NAME_IDX;
	data[index++] = (unsigned char) fileNameSize;
	memcpy(&data[index], al->fileName, fileNameSize);

	Packet packet;
	packet.data = data;
	packet.size = packetSize;

	printArray(packet.data, packet.size);
	p = &packet;

	//return llwrite(packet);
	return 0;
}

// mudar nome da função para algo maix explicativo
int fillArgs(uchar * data, ControlPacket * dest, int argNr, int argSize, int offset) {
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
	//Packet packet;
	/*
	if(llread(&packet) != OK)
		return ERROR;
	*/
	uchar * data = p->data;
	dest->type = data[CTRL_FIELD_IDX];

	dest->argNr = 0;
	int index = 1, argNr = 0, argSize;
	while(index < p->size){
		if(data[index++] != argNr)
			return ERROR;
		argSize = data[index++];
		if(fillArgs(data, dest, argNr, argSize, index) == ERROR)
			return ERROR;
		index += argSize;
		argNr++;
	}


	printf("Size: %d, Name: %s\n",dest->fileSize,dest->fileName);
	return OK;

}

int main() {
	al = (ApplicationLayer *) malloc(sizeof(ApplicationLayer));
	strcpy(al->fileName, "Derp.jpg");

	/*
	//DATA PACKET TEST
	unsigned char data[] = {0xFF, 0xFF, 0xFF, 0xBB, 0x1A};
	DataPacket dp, dr;
	dp.seqNr = 1;
	dp.size = 5;
	dp.data = data;
	sendDataPacket(&dp);
	receiveDataPacket(&dr);
	*/
	
	//CONTROL PACKET TEST
 	ControlPacket pi, pr;
	pi.fileSize = 5000000;
	pi.argNr = 2;
	sendControlPacket(&pi);
	receiveControlPacket(&pr);
	//printArray(packet->data, packet->size);
	//readControlPacket(packet, &pir);
	//printf("Filename: %s, Filesize: %d\n", pir.fileName, pir.fileSize);

	/*
	unsigned int val = 123456789;
	unsigned char value[4];
	convertIntToBytes(value, val);
	printf("Value: %0X\n",val);
	printArray(value,4);
	*/
	return 0;
}
