/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _IDXIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define BUF_SIZE 255
#define MSG_SIZE 5

#define MSG_FLAG 0x7E
#define MSG_A	0x03
#define SET_C	0x03
#define SET_BCC	(MSG_A ^ SET_C)
#define UA_C	0x07
#define UA_BCC	(MSG_A ^ UA_C)

#define FLAG1_IDX	0
#define A_IDX		1
#define C_IDX		2
#define BCC_IDX		3
#define FLAG2_IDX	4

#define TRIES	3

volatile int STOP=FALSE;

void emitter(int fd);
void receiver(int fd);

int main(int argc, char** argv)
{
	int fd,c, res;
	struct termios oldtio,newtio;
	char buf[BUF_SIZE];
	
	if ( (argc < 3) || 
			((strcmp("/dev/ttyS0", argv[1])!=0) && (strcmp("/dev/ttyS1", argv[1])!=0)) ||
			((strcmp("r", argv[2])!=0) && (strcmp("w", argv[2])!=0)) ) {
		printf("Usage:\tnserial SerialPort r/w\n\tex: nserial /dev/ttyS1 w\n");
		exit(1);
	}


/*
	Open serial port device for reading and writing and not as controlling tty
	because we don't want to get killed if linenoise sends CTRL-C.
*/


	fd = open(argv[1], O_RDWR | O_NOCTTY );
	if (fd <0) {perror(argv[1]); exit(-1); }

	if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 30;   /* inter-character timer unused - in 0.1s*/
	newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

/* 
	VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
	leitura do(s) pr�ximo(s) caracter(es)
*/

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	if (strcmp("r", argv[2]) == 0)
		receiver(fd);
	else if (strcmp("w", argv[2]) == 0)
		emitter(fd);
	else
		printf("Error in 3rd argument, should be 'r' or 'w'\n");

/*
* Reset terminal to previous configuration
*/
	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
	return 0;
}

void printArray(char buffer[], int size) {
	int i;
	for (i = 0; i < size; i++) {
		printf(" %02X ", (int) buffer[i]);
	}
	printf("\n");
}

int validUAMsg(char msg[]) {
	if(msg[FLAG1_IDX] != MSG_FLAG || msg[A_IDX] != MSG_A ||
		msg[C_IDX] != UA_C || msg[BCC_IDX] != (msg[A_IDX] ^ msg[C_IDX]) ||
		msg[FLAG2_IDX] != MSG_FLAG)
		return FALSE;
	return TRUE;
}

void emitter(int fd) {
	unsigned res;
	int i = 0;
	
	char writeBuffer[] = {MSG_FLAG, MSG_A, SET_C, SET_BCC, MSG_FLAG};
	char readBuffer[MSG_SIZE * sizeof(char)];
	
	// Emit SET
	while (i++ < TRIES) {
		printArray(writeBuffer, sizeof(writeBuffer));
		if ((res = write(fd, writeBuffer, sizeof(writeBuffer))) < sizeof(writeBuffer)) {
			printf("bad write: %d bytes\n", res);
			continue;
		}
		res = read(fd, readBuffer, sizeof(readBuffer));
		if (res < sizeof(readBuffer) || validUAMsg(readBuffer) == FALSE) {
			printf("bad read: %d bytes\n", res);
			continue;
		}
		printArray(readBuffer, sizeof(readBuffer));
		break;
	}
}

void writeUAMsg(int fd, char* buffer) {
	// Setting the UA Message
	buffer[FLAG1_IDX] = MSG_FLAG;
	buffer[A_IDX] = MSG_A;
	buffer[C_IDX] = UA_C;
	buffer[BCC_IDX] = UA_BCC;
	buffer[FLAG2_IDX] = MSG_FLAG;

	//Writting Message
	printArray(buffer, sizeof(buffer));
	int written = write(fd, buffer, 5);
	printf("written %d\n", written);
}

void receiver(int fd) {
	int num_received = 0, res;
	char buffer[MSG_SIZE];
	char c;

	while (num_received < MSG_SIZE) {				/* loop for input */
		res = read(fd, &c, 1);						/* returns each char */
		buffer[num_received++] = c;					/* printf the message later */
	}

	printArray(buffer, sizeof(buffer));

	/* Analizing the message received */
	int i;
	for (i = 0; i < num_received; i++) {
		switch(i) {
			case FLAG1_IDX:
			case FLAG2_IDX:
				if (buffer[i] != MSG_FLAG) {
					printf("Received Value of Flag different from expected. Value: 0x%x\n", buffer[i]);
					return;
				}
				break;
			case A_IDX:
				if (buffer[i] != MSG_A) {
					printf("Received Value of Adress Field different from expected. Value: 0x%x\n", buffer[i]);
					return;
				}
				break;
			case C_IDX:
				if (buffer[i] != SET_C) {
					printf("Received Value of Control Field different from expected. Value: 0x%x\n", buffer[i]);
					return;
				}
				break;
			case BCC_IDX:
				if (buffer[i] != SET_BCC) {
					printf("Received Value of Protection Field different from expected. Value: 0x%x\n", buffer[i]);
					return;
				}
				break;
			default:
				printf("Received extra: %x\n", buffer[i]);
		}
	}

	/* Re-writting the buffer with the UA message */
	writeUAMsg(fd, buffer); //Comment this line to try the time out mechanism
;}