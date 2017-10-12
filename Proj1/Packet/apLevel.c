#include <stdio.h>
#include "controlPacketInf.h"

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

void printArray(unsigned char buffer[], int size) {
	int i;
	for (i = 0; i < size; i++) {
		printf(" %02X ", buffer[i]);
	}
	printf("\n");
}

int array_append(unsigned char * base, int baseSize, unsigned char * suffix, int suffixSize){
	int totalSize = baseSize+suffixSize;
	if(totalSize > sizeof(base))
		return 1;
		
	int i, j = baseSize;
	for(i = 0; i < suffixSize; i++){
		base[j] = suffix[i];
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

unsigned char * makeDataPacket(unsigned char seqNr, int packetNr, unsigned char * buffer){
	int packetSize = HEADER_SIZE+packetNr;
	unsigned char packet[packetSize];
	
	packet[CTRL_FIELD] = DATA_VAL;
	packet[SEQ_NUM] = seqNr;
	packet[DATA_PACKET_SIZE2] = (unsigned char) (packetNr/SIZE);
	packet[DATA_PACKET_SIZE1] = (unsigned char) (packetNr%SIZE);
	
	int i, j = HEADER_SIZE;
	for(i = 0; i < sizeof(buffer);i++){
		packet[j] = buffer[i];
		j++;
	}

	printArray(packet,sizeof(packet));

	return packet;
}

unsigned char * makeControlPacket(unsigned char type, struct ControlPacketInf * controlPacketInf){
	int fileNameSize = sizeof(controlPacketInf->fileName);
	int packetSize = 1+2*controlPacketInf->argNr+fileNameSize+FILE_SIZE_LENGTH;
	
	unsigned char packet[packetSize];
	
	int index = 1;
	packet[CTRL_FIELD] = type;
	
	packet[index++] = FILE_SIZE;
	packet[index++] = FILE_SIZE_LENGTH;
	unsigned char fileSize[4];
	convertIntToBytes(fileSize, controlPacketInf->fileSize);
	
	array_append(packet, index, fileSize, FILE_SIZE_LENGTH);
	index += FILE_SIZE_LENGTH;
	
	packet[index++] = FILE_NAME;
	packet[index++] = (unsigned char) fileNameSize;
	array_append(packet, index, controlPacketInf->fileName, fileNameSize);
	
	return packet;
}

int main(){
	
	unsigned char data[] = {0xFF, 0xFF, 0xFF, 0xBB, 0x1A};
	unsigned char * packet = makeDataPacket(1,5, data);
	printArray(packet, sizeof(packet));
	/*
	unsigned int val = 123456789;
	unsigned char value[4];
	convertIntToBytes(value, val);
	printf("Value: %0X\n",val);
	printArray(value,4);
	*/
	return 0;
}