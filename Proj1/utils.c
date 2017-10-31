#include <termios.h>
#include "utils.h"

int DEBUG = FALSE;

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
void convertIntToBytes(uchar * res, uint src) {
	int i;
	for(i = sizeof(int)-1; i >= 0; i--) {
		res[i] = (unsigned char) src;
		src >>= 8;
	}
}

uint convertBytesToInt(uchar * src) {
	uint i, offset = 24;
	uint res = 0;
	for(i = 0; i < sizeof(uint); i++) {
		res |= ((uint) src[i]) << offset;;
		offset -= 8;
	}
	return res;
}

int logError(char * errorMsg) {
 	if (DEBUG) {
		fprintf(stderr, "Error: %s\n", errorMsg);
	}

	return ERROR;
}

int getBaudrate(int baudrate) {
	switch (baudrate) {
	case 0:
		return B0;
	case 50:
		return B50;
	case 75:
		return B75;
	case 110:
		return B110;
	case 134:
		return B134;
	case 150:
		return B150;
	case 200:
		return B200;
	case 300:
		return B300;
	case 600:
		return B600;
	case 1200:
		return B1200;
	case 1800:
		return B1800;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	default:
		return -1;
	}
}

const int PROGRESS_BAR_LENGTH = 40;

void printProgressBar(int current, int total) {
	float percentage = 100.0 * (float) current / (float) total;

	printf("\rCompleted: %6.2f%% |", percentage);

	int i, len = PROGRESS_BAR_LENGTH;
	int pos = percentage * len / 100.0;

	for (i = 0; i < len; i++)
		i <= pos ? printf("■") : printf(" ");

	printf("|");

	fflush(stdout);
}
