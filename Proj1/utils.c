#include "utils.h"

long getFileSize(FILE* file) {
	// saving current position
	long currentPosition = ftell(file);

	// seeking end of file
	if (fseek(file, 0, SEEK_END) == -1) {
		printf("ERROR: Could not get file size.\n");
		return -1;
	}

	// saving file size
	long size = ftell(file);

	// seeking to the previously saved position
	fseek(file, 0, currentPosition);

	// returning size
	return size;
}

void printArray(uchar buffer[], int size) {
	int i;
	for (i = 0; i < size; i++) {
		printf("%02X ", buffer[i]);
	}
	printf("\n");
}

// useless function pls
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

int logError(char * errorMsg){
	fprintf(stderr, "Error: %s\n", errorMsg);
	return ERROR;
}