#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// TODO filtrar estes includes

#define FLAG 	0x7E
#define ESC 	0x7D
#define TRUE	0
#define FALSE	1

// Control Fields
#define C_SET	0x03
#define C_DISC	0x0B
#define C_UA	0x07
#define C_RR1	0x85
#define C_RR0	0x05
#define	C_REJ1	0x81
#define C_REJ0	0x01
#define C_INF1	0x40
#define C_INF0	0x00

//Protection Field
#define BCC1_SET

//static struct linkLayer globalPtr;

typedef unsigned int uint;


int byteStuffing(char * buffer, uint * size) {
	uint i;

	for (i = 0; i < (*size); ++i) {
		if ((buffer[i] == (char) FLAG) || (buffer[i] == (char) ESC)) 
		{
			if ((buffer = realloc(buffer, (*size)++)) == NULL) {
				printf("ByteStuffing: Realloc error.\n");
				return FALSE;
			}

			memmove(buffer+i+1, buffer+i, (*size)-i);
			buffer[i] = ESC;
			i++;
		}
	}

	return TRUE;
}

int byteDestuffing(char* buffer, int* size) {
	uint i;

	for (i = 0; i < (*size); ++i) {
		if (buffer[i] == (char) ESC)
		{
			memmove(buffer+i, buffer+i+1, (*size)-i);
			buffer = realloc(buffer, (*size)--);
			if (buffer == NULL) {
				printf("ByteDestuffing: Realloc error.\n");
				return FALSE;
			}
		}
	}

	return TRUE;
}

int addHeader(char* buffer, int *length) {

}


int llwrite(int fd, char * buffer, int length) {
	//adição de stuffing ao buffer
	//adição de header e trailer ao buffer.
	//escrever a nova mensagem
}

int llread(int fd, char * buffer) {
	// adição de header e trailer ao buffer.
	//remoção de stuffing ao buffer
	//escrever a nova mensagem
}

int main() {

	unsigned char* buffer = malloc(8);
	buffer [0] = 0x01;
	buffer [1] = 0x02; 
	buffer [2] = 0x03; 
	buffer [3] = 0x7E; 
	buffer [4] = 0x04; 
	buffer [5] = 0x05; 
	buffer [6] = 0xCC; 
	buffer [7] = 0x06; 

	uint len = sizeof(buffer);

	printf("\n\ninitial size: %d\n", len);

	byteStuffing(buffer, &len);

	printf("\nmed size: %d\n", len);

	uint i;
	for (i = 0; i < len; ++i) {
		printf("0x%x  ", (uint) buffer[i]);
	}
	printf("\n");
	byteDestuffing(buffer, &len);

	int j;
	for (j = 0; j < len; ++j) {
		printf("0x%x  ", buffer[j]);
	}
	printf("\n\nSize: %d\n", len);
}
