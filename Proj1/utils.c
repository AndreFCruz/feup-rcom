#include "utils.h"

void printArray(uchar buffer[], int size) {
	int i;
	for (i = 0; i < size; i++) {
		printf("%02X ", buffer[i]);
	}
	printf("\n");
}

void convertIntToBytes(uchar * res, uint src){
	int i;
	for(i = sizeof(int)-1; i >= 0; i--){
		res[i] = (unsigned char) src;
		src >>= 8;
	}
}

uint convertBytesToInt(uchar * src){
	int i, offset = 24;
	uint res = 0;
	for(i = 0; i < sizeof(int); i++){
		res |= ((uint) src[i]) << offset;;
		offset -= 8; 
	}
	return res;
}