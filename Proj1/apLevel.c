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


//TODO: use memcpy

void printArray(unsigned char buffer[], int size) {
	int i;
	for (i = 0; i < size; i++) {
		printf("%02X ", buffer[i]);
	}
	printf("\n");
}

int array_append(unsigned char * base, int baseSize, int baseStart, unsigned char * suffix, int suffixSize){
	int totalSize = baseStart+suffixSize;
	//printf("TotalSize: %d, BaseSize: %d\n",totalSize, sizeof(base));

	if(totalSize > baseSize)
		return 1;

	int i, j = baseStart;
	for(i = 0; i < suffixSize; i++){
		base[j] = suffix[i];
		//printf("Suffix: %d\n",suffix[i]);
		j++;
	}

	return 0;
}

void convertIntToBytes(unsigned char * value, unsigned int val){
	int i;
	for(i = sizeof(int)-1; i >= 0; i--){
		value[i] = (unsigned char) val;
		val >>= 8;
	}
}

Packet * makeDataPacket(unsigned char seqNr, int packetNr, unsigned char * buffer){
	int packetSize = HEADER_SIZE+packetNr;
	unsigned char * data = (unsigned char *) malloc(packetSize);

	data[CTRL_FIELD] = DATA_VAL;
	data[SEQ_NUM] = seqNr;
	data[DATA_PACKET_SIZE2] = (unsigned char) (packetNr/SIZE);
	data[DATA_PACKET_SIZE1] = (unsigned char) (packetNr%SIZE);

	array_append(data, packetSize, HEADER_SIZE, buffer, packetNr);

	Packet * packet = (Packet *) malloc(sizeof(Packet));
	packet->data = data;
	packet->size = packetSize;

	return packet;
}

Packet * makeControlPacket(unsigned char type, ControlPacketInf * controlPacketInf){
	int fileNameSize = strlen(controlPacketInf->fileName);
	int packetSize = 1+2*controlPacketInf->argNr+fileNameSize+FILE_SIZE_LENGTH;
	//printf("packetSize: %d, fileNameSize: %d\n", packetSize, (unsigned char) fileNameSize);

	unsigned char * data = (unsigned char *) malloc(packetSize);

	int index = 1;
	data[CTRL_FIELD] = type;

	data[index++] = FILE_SIZE;
	data[index++] = FILE_SIZE_LENGTH;
	unsigned char fileSize[sizeof(int)];
	convertIntToBytes(fileSize, controlPacketInf->fileSize);

	array_append(data, packetSize, index, fileSize, FILE_SIZE_LENGTH);
	index += FILE_SIZE_LENGTH;

	data[index++] = FILE_NAME;
	data[index++] = (unsigned char) fileNameSize;
	array_append(data, packetSize, index, controlPacketInf->fileName, fileNameSize);

	Packet * packet = (Packet *) malloc(sizeof(Packet));
	packet->data = data;
	packet->size = packetSize;

	return packet;
}

int readControlPacket(Packet * packet, ControlPacketInf * packetInf){
	unsigned char * data = packet->data;
	int i=1;
	/*
	t = data[i++];
	l = data[i++];
	*/
	i += 2;
	packetInf->fileSize = (int) data[i];
	i += HEADER_SIZE+1;
	/*
	t = data[i++];
	l = data[i++];
	*/
	i += 2;
	strcpy(packetInf->fileName, (char) data[i]);
	printf("Filename: %s\n", packetInf->fileName);
	return 0;
}

int main(){
	/*
	//DATA PACKET TEST
	unsigned char data[] = {0xFF, 0xFF, 0xFF, 0xBB, 0x1A};
	Packet * packet = makeDataPacket(1,5, data);
	printArray(packet->data, packet->size);
	free(packet);
	*/

	//CONTROL PACKET TEST
 	ControlPacketInf pi, pir;
	pi.fileSize = 5000;
	strcpy(pi.fileName, "Derp.jpg");
	pi.argNr = 2;
	Packet * packet = makeControlPacket(2, &pi);
	printArray(packet->data, packet->size);
	readControlPacket(packet, &pir);
	printf("Filename: %s, Filesize: %d\n", pir.fileName, pir.fileSize);
	free(packet);


	/*
	unsigned int val = 123456789;
	unsigned char value[4];
	convertIntToBytes(value, val);
	printf("Value: %0X\n",val);
	printArray(value,4);
	*/
	return 0;
}
