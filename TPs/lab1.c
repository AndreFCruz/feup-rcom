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
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define BUF_SIZE 255

volatile int STOP=FALSE;

void emissor(int fd);
void recetor(int fd);

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

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

/* 
	VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
	leitura do(s) próximo(s) caracter(es)
*/

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	printf("New termios structure set\n");

	if (strcmp("r", argv[2]) == 0)
		recetor(fd);
	else if (strcmp("w", argv[2]) == 0)
		emissor(fd);
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

void emissor(int fd) {
	int i;
	char buffer[BUF_SIZE];

	// get input from stdin
	if (gets(buffer) == NULL) {
		perror("gets");
		exit(-1);
	}
	int n = strnlen(buffer, BUF_SIZE) + 1;
	
	// write to serial port
	int res = write(fd, buffer, n);
	printf("%d bytes read : %s\n", n, buffer);
	printf("%d bytes written\n", res);

	// clear buffer
	memset(buffer, 0, BUF_SIZE);

	// read feedback, 8 bytes at a time
	for(i = 0; i < BUF_SIZE; i += n) {
		n = read(fd, buffer + i, BUF_SIZE);
			printf("\n%d bytes read : %s\n", n, &buffer[i]);
		if (n < 8 || buffer[i+n-1] == 0)
			break;
	}

	printf("\n read: %s\n", buffer);
}

void recetor(int fd) {
	int i = 0, res;
	char buffer[BUF_SIZE];
	char c;

	while (STOP==FALSE) {				/* loop for input */
		res = read(fd, &c, 1);		/* returns each char */
		buffer[i++] = c;					/* printf the message later */
		if (c == 0)
			STOP=TRUE; 
	}

	/* Printing the string passed in the serial */
	printf("\n%s\n", buffer);

	/* Re writing the message through the serial */
	printf("\n%i\n", i);
	write(fd, buffer, i);
}