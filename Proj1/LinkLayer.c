#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "LinkLayer.h"

#define FLAG 			0x7E
#define ESC 			0x7D
#define STUFFING 		0X20

#define CONTROL_FRAME_SIZE	5
#define INF_FORMAT_SIZE		6
#define INF_HEAD_SIZE		4
#define INF_TRAILER_SIZE 	2
#define RECEIVER_SIZE		256

//Information Message Trailers position
#define TRAIL_BCC_POS		0
#define TRAIL_FLAG_POS		1

//Positions of Fields in the Control Message
#define FLAG1_POS	1
#define AF_POS		2
#define CF_POS 			3
#define BCC_POS 	4
#define FLAG2_POS	5

//Possible Adress Fields
#define AF1		0x03
#define AF2		0x01

// Control Fields
#define C_SET	0x03
#define C_DISC	0x0B
#define C_UA	0x07
#define C_RR	0x05
#define C_REJ	0x01
#define C_INF	0x00

#define MAX_PORT_NAME 32

typedef struct {
	char port[MAX_PORT_NAME];
	int baudRate;
	uint seqNumber;
	uint timeout;
	uint numTrans;
} LinkLayer;

static LinkLayer * ll = NULL;

static struct termios oldtio;

typedef enum
{SET, DISC, UA, RR, REJ} ControlType;

//TODO: REVER ISTO AQUI -> O R TEM QUE SER CONTROLADO DE ALGUMA FORMA É COM O SEQUENCE NUMBER (seqNumber) SEU BUBA
int nr = 0;
int ns = 0; // rip dis shit

/**
 * Creates a Control Frame, according to the protocols.
 *
 * @param adressField The Adress field value.
 * @param controlField The Control field value.
 * @return The generated frame.
 *//*
char * createControlFrame(char * buffer, char adressField, char controlField) {
	char * buffer = malloc(CONTROL_FRAME_SIZE);

	buffer[FLAG1_POS] = FLAG;
	buffer[AF_POS] = adressField;
	buffer[CF_POS] = controlField;
	buffer[BCC_POS] = (adressField ^ controlField);
	buffer[FLAG2_POS] = FLAG;

	return buffer;
}*/

/**
 * Generates a Controll frame, corresponding to the given type.
 *
 * @param type The type of control frame TODO - MUDAR ESTA FUNAÇÃO PARA DEIXAR DE DEPENDER DA CONNECTIONTYPE
 * @return The frame generated.
 */ /*
char * genControlFrame(ControlType controlType, ConnectionType connType) {
	int nrValue = (nr << 7);

	if (connType == TRANSMITER) {
		switch(controlType) {
			case SET:
				return createControlFrame(AF1, C_SET);
			case DISC:
				return createControlFrame(AF1, C_DISC);
			case UA:
				return createControlFrame(AF2, C_UA);
			case RR:
				return createControlFrame(AF2, (C_RR | nrValue));
			case REJ:
				return createControlFrame(AF2, (C_REJ | nrValue));
		}
	}
	else if (connType == RECEIVER) {
		switch(controlType) {
			case SET:
				return createControlFrame(AF2, C_SET);
			case DISC:
				return createControlFrame(AF2, C_DISC);
			case UA:
				return createControlFrame(AF1, C_UA);
			case RR:
				return createControlFrame(AF1, (C_RR | nrValue));
			case REJ:
				return createControlFrame(AF1, (C_REJ | nrValue));
		}
	}
	return NULL;
} */

/**
 * Evaluates if the Frame's header is wrong, being descarted if so.
 *
 * @param frame The frame to evaluated
 * @param size The frame's size.
 * @return ERROR if something went wrong, OK otherwise
 */ /*
int evaluateFrameHeader(char* frame, char* size) {
	//Checking the Flag field
	if (frame[FLAG1_POS] != FLAG) {
		printf("Error in received Frame Header: Flag Field\n");
		return ERROR;
	}

	//Checking the Adress Field
	if ((frame[AF_POS] != AF1) || (frame[AF_POS] != AF2)) {
		printf("Error in received Frame Header: Adress Field\n");
		return ERROR;
	}

//TODO puxar para outra funcao validFrame
	//Checking the Control Field
	if ((frame[CF_POS] != C_SET) ||
		(frame[CF_POS] != C_DISK) ||
		(frame[CF_POS] != C_UA) ||
		(frame[CF_POS] != (C_RR | nr)) ||
		(frame[CF_POS] != (C_RR | ~nr)) ||
		(frame[CF_POS] != (C_REJ| nr)) ||
		(frame[CF_POS] != (C_REJ| ~nr)) ||
		(frame[CF_POS] != (C_INF| ns)) ||
		(frame[CF_POS] != (C_INF| ~ns))) {
		printf("Error in received Frame Header: Control Field\n");
		return ERROR;
	}

	//Checking the Protection Field
	if ((frame[AF_POS] ^ frame[CF_POS]) != frame[BCC_POS]) {
		printf("Error in received Frame Header: Protection Field\n");
		return ERROR;
	}

	return OK;
} */

/**
 * Reads a frame and checks if it is of TYPE == type
 *
 * @return OK if it was, ERROR otherwise.
 */ 
int receiveControlFrame(int fd, ControlType type) {
	//TODO
	return ERROR;
}

/**
 * Creates a Information Frame, according to the protocols.
 * Final Frame and its size its retrivied in the function parameters.
 *
 * @param packet The packet to be framed.
 * @param size The packet's size.
 * @return ERROR if something went wrong, OK otherwise
 */ /*
int createInfFrame(char* packet, uint* size) {

	int previousSize = (*size);
	(*size) += INF_FORMAT_SIZE;

	if ((packet = realloc(packet, (*size))) == NULL) {
		printf("createInfFrame: Realloc error.\n");
		return ERROR;
	}

	//Setting the trailer
	packet[(*size) + TRAIL_BCC_POS] = 0; */
	/* TODO: BCC (Block Check Character) – detecção de erros baseada na geração de
um octeto (BCC) tal que exista um número par de 1s em cada posição
(bit), considerando todos os octetos protegidos pelo BCC (cabeçalho ou
dados, conforme os casos) e o próprio BCC (antes de stuffing) */ /*
	packet[(*size) + TRAIL_FLAG_POS] = FLAG;

	memmove(packet + INF_HEAD_SIZE, packet, previousSize + INF_TRAILER_SIZE);
	packet[FLAG1_POS] = FLAG;
	packet[AF_POS] = AF1;
	packet[CF_POS] = (C_INF | (ns << 6));
	packet[BCC_POS] = (AF1 ^ packet[CF_POSF]);

	return OK;
} */

/**
 * Applies byte stuffing to the given message according to the protocols, retriving the new message in the same buffer.
 *
 * @param buffer The buffer containing the message to be stuffed.
 * @param size The buffer's size
 * @return ERROR if something went wrong, OK otherwise
 */
int byteStuffing(char * buffer, uint * size) {
	uint i;

	for (i = 0; i < (*size); ++i) {
		if ((buffer[i] == (char) FLAG) || (buffer[i] == (char) ESC))
		{
			if ((buffer = realloc(buffer, (*size)++)) == NULL) {
				printf("ByteStuffing: Realloc error.\n");
				return ERROR;
			}

			memmove(buffer+i+1, buffer+i, (*size)-i);
			buffer[i] = ESC;
			buffer[i+1] = (buffer[i+1] ^ STUFFING);
			i++;
		}
	}

	return OK;
}

/**
 * Applies byte destuffing to the given message according to the protocols, retriving the new message in the same buffer.
 *
 * @param buffer The buffer containing the stuffed message.
 * @param size The buffer's size
 * @return Error if something went wrong, ok otherwise
 */
int byteDestuffing(char* buffer, uint * size) {
	uint i;

	for (i = 0; i < (*size); ++i) {
		if (buffer[i] == (char) ESC)
		{
			memmove(buffer+i, buffer+i+1, (*size)-i);
			buffer = realloc(buffer, (*size)--);
			if (buffer == NULL) {
				printf("ByteDestuffing: Realloc error.\n");
				return ERROR;
			}

			buffer[i] = (buffer[i] ^ STUFFING);
		}
	}

	return OK;
}

int llwrite(int fd, char * buffer, int length) {
	int res = 0;

	/*if (createInfFrame(buffer, &length) == ERROR) {
		printf("llwrite error: Failed to create Information Frame.\n");
		return -1;
	}*/

	if (byteStuffing(buffer, &length) == ERROR) {
		printf("llwrite error: Failed to create Information Frame.\n");
		return -1;
	}

	do {
		if ((res = write(fd, buffer, length) < length)) {
			printf("llwrite error: Bad write: %d bytes\n", res);
			return -1;
		}
	} while (receiveControlFrame(fd, RR));

	return res;
}

//Retorna o n de caracteres lidos total
int readFrameFlag(int fd) {
	char tempChar;
	int readBytes = 0;

	while (tempChar != FLAG) {
		if ( (tempChar = read(fd, &tempChar, sizeof(char))) < 0 ) {
			printf("readFrameFlag error: Failed to read from SerialPort\n");
			return -1;
		}
		++readBytes;
	}
	return readBytes;
}

int llread(int fd, char ** dest) {
	char * buffer = malloc(RECEIVER_SIZE);
	int bufferIdx = 0;		//Number of bytes received

	if (readFrameFlag(fd) < 0) {
		printf("llread Error: read Frame flag error\n");
		return -1;
	}

	while (buffer[bufferIdx] != FLAG) {
		if (read(fd, buffer + bufferIdx, sizeof(char)) < 0) {
			printf("llread error: Failed to read from SerialPort\n");
			return -1;
		}

		if ((++bufferIdx % RECEIVER_SIZE) == 0 ) {
			if ((buffer = realloc(buffer, ((bufferIdx / RECEIVER_SIZE)+1) * RECEIVER_SIZE )) == NULL) {
				printf("llread error: Failed to realloc buffer\n");
				return -1;
			}
		}
	}

	if (byteDestuffing(buffer, &bufferIdx) == ERROR) {
		printf("llread error: Failed byteDestuffing\n");
		return -1;
	}
	*dest = buffer;

	//TODO: Retirar o head e o trailer
	//write(fd, genControlFrame(RR), CONTROL_FRAME_SIZE);
	return bufferIdx;
}

int initLinkLayer(char port[], int baudRate, uint timeout, uint numTransmissions) {
	ll = malloc(sizeof(LinkLayer));

	ll->port = port;
	ll->baudRate = baudRate;
	ll->timeout = timeout;
	ll->numTrans = numTransmissions;
	ll->seqNumber = 0;
}

int llopen(int porta, ConnectionType type) {
	int fd,c, res;
	struct termios newtio;

	ll->seqNumber = (type == TRANSMITTER ? 0 : 1);

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
	newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */

/*
	VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
	leitura do(s) proximo(s) caracter(es)
*/

	tcflush(fd, TCIOFLUSH);

	if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}
	printf("New termios structure set\n");

//sE TRANSMITTER MANDA, se ERCEIVER espera pela set.
	//TODO: Mandar uma mensagem de contol: SET
}

int llclose(int fd, ConnectionType type) {

	//TODO mandar control message -> disc, que varia para read e writter

	/*
	* Reset terminal to previous configuration
	*/
		if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
			perror("tcsetattr");
			exit(-1);
		}

		close(fd);
		return OK;
}


int main() {

	unsigned char* buffer = malloc(8);
	buffer [0] = 0x01;
	buffer [1] = 0x02;
	buffer [2] = 0x03;
	buffer [3] = 0x7E;
	buffer [4] = 0x04;
	buffer [5] = 0x05;
	buffer [6] = 0x7D;
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
