#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ApplicationLayer.h"

#define DATA_VAL		1
#define START_VAL		2
#define END_VAL			3


#define SIZE			256     //mudar nome de variavel
#define HEADER_SIZE		4


//DATA_PACKET
#define CTRL_FIELD 			0
#define SEQ_NUM				1
#define DATA_PACKET_SIZE2	2
#define DATA_PACKET_SIZE1	3

//CONTROL_PACKET
#define FILE_SIZE		0
#define FILE_NAME		1

#define FILE_SIZE_LENGTH	4

ApplicationLayer * al;
Packet * p;

int sendDataPacket(DataPacket * src){
	int packetSize = HEADER_SIZE+src->size;
	unsigned char * data = (unsigned char *) malloc(packetSize);

	data[CTRL_FIELD] = DATA_VAL;
	data[SEQ_NUM] = src->seqNr;
	data[DATA_PACKET_SIZE2] = (unsigned char) (src->size/SIZE);
	data[DATA_PACKET_SIZE1] = (unsigned char) (src->size%SIZE);

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
	if(data[CTRL_FIELD] != 1)
		return ERROR;
	dest->seqNr = data[SEQ_NUM];
	int size = data[DATA_PACKET_SIZE2]*SIZE+data[DATA_PACKET_SIZE1];
	dest->data = (uchar *) malloc(size);
	memcpy(dest->data, &data[HEADER_SIZE], size);
	free(dest->data);
	return OK;
}


int sendControlPacket(ControlPacket * src){
	int fileNameSize = strlen(al->fileName);
	int packetSize = 1+2*src->argNr+fileNameSize+FILE_SIZE_LENGTH;
	//printf("packetSize: %d, fileNameSize: %d\n", packetSize, (unsigned char) fileNameSize);

	unsigned char * data = (unsigned char *) malloc(packetSize);

	int index = 1;
	data[CTRL_FIELD] = 2;

	data[index++] = FILE_SIZE;
	data[index++] = FILE_SIZE_LENGTH;
	unsigned char fileSize[sizeof(int)];
	convertIntToBytes(fileSize, src->fileSize);
	memcpy(&data[index], fileSize, FILE_SIZE_LENGTH);

	printf("Size: %d\n", src->fileSize);
	//memcpy(&data[index], (uchar*) &src->fileSize, sizeof(int));

	index += FILE_SIZE_LENGTH;

	data[index++] = FILE_NAME;
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

int fillArgs(uchar * data, ControlPacket * dest, int argNr, int argSize, int offset){
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

int receiveControlPacket(ControlPacket * dest){
	//Packet packet;
	/*
	if(llread(&packet) != OK)
		return ERROR;
	*/
	uchar * data = p->data;
	if(data[CTRL_FIELD] == START_VAL)
		dest->type = START;
	else 
		dest->type = END;

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

int main(){
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
