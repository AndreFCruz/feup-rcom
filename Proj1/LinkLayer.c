#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "LinkLayer.h"

#define FLAG 				0x7E
#define ESC 				0x7D
#define STUFFING 			0X20

#define CONTROL_FRAME_SIZE	5
#define INF_FORMAT_SIZE		6
#define INF_HEAD_SIZE		4
#define INF_TRAILER_SIZE 	2
#define RECEIVER_SIZE		256

//Information Message Trailers position
#define TRAIL_BCC_POS		0
#define TRAIL_FLAG_POS		1

//Positions of Fields in the Control Message
#define FLAG1_POS			1
#define AF_POS				2
#define CF_POS 				3
#define BCC_POS 			4
#define FLAG2_POS			5

//Possible Adress Fields
#define AF1					0x03
#define AF2					0x01

// Control Fields
#define C_SET				0x03
#define C_DISC				0x0B
#define C_UA				0x07
#define C_RR				0x05
#define C_REJ				0x01
#define C_INF				0x00

#define MAX_PORT_NAME 		16
#define PORT_NAME			"/dev/ttyS"

typedef struct {
	char port[MAX_PORT_NAME];
	int baudRate;
	uint seqNumber;
	uint timeout;
	uint numTrans;
} LinkLayer;

static LinkLayer * ll = NULL;

static struct termios oldtio;

typedef enum {
	SET = 0x03, DISC = 0x0B, UA = 0x07, RR = 0x05, REJ = 0x01
} ControlType;


/**
 * Keeps reading until a FRAME FLAG is found.
 *
 * @param The Serial Port's filedescriptor
 * @returned Number of bytes read, -1 if an error ocurred.
 */
int readFrameFlag(int fd);

/**
 * Creates a Control Frame, according to the protocols.
 *
 * @param buffer Buffer where the control Frame will be created
 * @param adressField The Adress field value.
 * @param controlField The Control field value.
 * @return OK if all went well, ERROR otherwise
 */
void createControlFrame(char * buffer, char adressField, char controlField);

/**
 * Sends a Control frame of the given type.
 *
 * @param fd The Serial Port filedescriptor
 * @param controlType The type of control frame
 * @return Number of written bytes, -1 if an error happened.
 */
int sendControlFrame(int fd, ControlType controlType);

/**
 * Sends a Control frame of the given type, with the given adress field
 * For the special occasion of variating Adress Fields from Transmisser / Receiver
 *
 * @param fd The Serial Port filedescriptor
 * @param controlType The type of control frame
 * @param adressField The adress field to be used
 * @return Number of written bytes, -1 if an error happened.
 */
int sendControlFrame(int fd, ControlType controlType, char adressField);

/**
 * Reads a frame and checks if it is of the given type.
 *
 * @param The Serial Port's filedescriptor
 * @param controlType The type of control frame
 * @return OK if the frame was of the given type, ERROR otherwise.
 */ 
int readControlFrame(int fd, ControlType controlType);

/**
 * Reads a frame and checks if it is of the given type, and has the given adress field
 * For the special occasion of variating Adress Fields from Transmisser / Receiver
 *
 * @param The Serial Port's filedescriptor
 * @param controlType The type of control frame
 * @param adressField The adress field to be used
 * @return OK if the frame was of the given type, ERROR otherwise.
 */ 
int readControlFrameWAdress(int fd, ControlType controlType, char adressField);

/**
 * Creates a Information Frame, according to the protocols.
 * Final Frame and its size its retrivied in the function parameters.
 *
 * @param packet The packet to be framed.
 * @param size The packet's size.
 * @return ERROR if something went wrong, OK otherwise
 */ 
int framingInformation(char* packet, uint* size);

/**
 * Evaluates if the framing is wrong, being descarted if so.
 * The deframed frame and its size are returned in the given arguments.
 *
 * @param frame The frame to be evaluated
 * @param size The frame's size.
 * @return ERROR if something went wrong, OK otherwise
 */ 
int deframingInformation(char * frame, uint* size);

/**
 * Applies byte stuffing to the given message according to the protocols, retriving the new message in the same buffer.
 *
 * @param buffer The buffer containing the message to be stuffed.
 * @param size The buffer's size
 * @return ERROR if something went wrong, OK otherwise
 */
int byteStuffing(char * buffer, uint * size);

/**
 * Applies byte destuffing to the given message according to the protocols, retriving the new message in the same buffer.
 *
 * @param buffer The buffer containing the stuffed message.
 * @param size The buffer's size
 * @return Error if something went wrong, ok otherwise
 */
int byteDestuffing(char* buffer, uint * size);



int initLinkLayer(int porta, int baudRate, uint timeout, uint numTransmissions) {
	ll = malloc(sizeof(LinkLayer));

	snprintf(ll->port, MAX_PORT_NAME, "%s%d", PORT_NAME, porta);

	ll->baudRate = baudRate;
	ll->timeout = timeout;
	ll->numTrans = numTransmissions;
	ll->seqNumber = 0;
}

int openSerialPort() {
	if (ll == NULL)
		return logError("LinkLayer not initialized");

	int fd = open(ll->port, O_RDWR | O_NOCTTY );
	if (fd <0) { perror(ll->port); exit(-1); }

	if ( tcgetattr(fd,&oldtio) == -1 ) { /* save current port settings */
		perror("tcgetattr");
		exit(-1);
	}

	struct termios newtio;
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = ll->baudRate | CS8 | CLOCAL | CREAD;
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

	return fd;
}

int llopen(ConnectionType type) {
	int fd, c, res;
	struct termios newtio;

	ll->seqNumber = (type == TRANSMITTER ? 0 : 1);
	fd = openSerialPort();

	if (type == TRANSMITTER) {
		sendControlFrame(fd, SET);
		readControlFrame(fd, UA);
		return fd;
	} 
	else if (type == RECEIVER) {
		readControlFrame(fd,SET);
		sendControlFrame(fd, UA);
		return fd;
	} 
	else return logError("Unknow Connection Type");
}

int llclose(int fd, ConnectionType type) {

	if (type == TRANSMITTER) {			//TODO, verificar o resultado das funções?
		sendControlFrame(fd, DISC);
		readControlFrameWAdress(fd, DISC, AF2);
		sendControlFrame(fd, UA, AF2);
	} else {
		readControlFrame(fd, DISC);
		sendControlFrame(fd, DISC, AF2);
		readControlFrameWAdress(fd, UA, AF2);
	}


	//Reset terminal to previous configuration
	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
	return OK;
}

int llwrite(int fd, char * buffer, int length) {
	int res = 0;

	if (framingInformation(buffer, &length) == ERROR) {
		printf("llwrite error: Failed to create Information Frame.\n");
		return -1;
	}

	if (byteStuffing(buffer, &length) == ERROR) {
		printf("llwrite error: Failed to create Information Frame.\n");
		return -1;
	}

	do {
		if ((res = write(fd, buffer, length) < length)) {
			printf("llwrite error: Bad write: %d bytes\n", res);
			return -1;
		}
	} while (readControlFrame(fd, RR) == OK);

	return res;
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

	if (deframingInformation(buffer, &bufferIdx) != OK)
		return logError("Failed to deframe information");

	*dest = buffer;

	sendControlFrame(fd, RR);
	return bufferIdx;
}



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


void createControlFrame(char * buffer, char adressField, char controlField) {
	buffer = malloc(CONTROL_FRAME_SIZE);

	buffer[FLAG1_POS] = FLAG;
	buffer[AF_POS] = adressField;
	buffer[CF_POS] = controlField;
	buffer[BCC_POS] = (adressField ^ controlField);
	buffer[FLAG2_POS] = FLAG;
}

int sendControlFrame(int fd, ControlType controlType) {
	int nrValue = (ll->seqNumber << 7);

	char * controlFrame = NULL;
	if ((controlType == RR) || (controlType ==REJ)) {
		createControlFrame(controlFrame, AF1, (controlType | nrValue));
	} else {
		createControlFrame(controlFrame, AF1, controlType);
	}

	int res;
	if ((res = write(fd, controlFrame, CONTROL_FRAME_SIZE)) < CONTROL_FRAME_SIZE) {
		printf("senControlFrame error: Bad write: %d bytes\n", res);
		return -1;
	}
	
	free(controlFrame);
	return res;
}

int sendControlFrame(int fd, ControlType controlType, char adressField) {
	
	if ((controlType != DISC) && (controlType != UA))
		sendControlFrame(fd, controlType);

	char * controlFrame = NULL;
	createControlFrame(controlFrame, adressField, controlType);

	int res;
	if ((res = write(fd, controlFrame, CONTROL_FRAME_SIZE)) < CONTROL_FRAME_SIZE) {
		printf("senControlFrame (Disk) error: Bad write: %d bytes\n", res);
		return -1;
	}
	
	free(controlFrame);
	return res;
}

int readControlFrame(int fd, ControlType controlType) {
	char * controlFrame = malloc(CONTROL_FRAME_SIZE);

	if (read(fd, controlFrame, CONTROL_FRAME_SIZE) < CONTROL_FRAME_SIZE)
		return logError("Failed to read Control Frame");

	if ((controlFrame[FLAG1_POS] == FLAG) && (controlFrame[CF_POS] == controlType) && (controlFrame[AF_POS] == AF1) && 
		(controlFrame[FLAG2_POS] == FLAG) && (controlFrame[BCC_POS] == (controlFrame[AF_POS] ^ controlFrame[CF_POS]))) 
	{
		if ((controlType == RR) || (controlType == REJ)) {
			if (controlFrame[CF_POS] == (controlType | (ll->seqNumber << 7) )) //TODO: ll->receivedSeqNumber
				return OK;
		
		} else {
			if (controlFrame[CF_POS] == controlType)
				return OK;
		}
	}
	free(controlFrame);
	return logError("Frame was note of the given type or Flags were not recognized\n");
}

int readControlFrameWAdress(int fd, ControlType controlType, char adressField) {
	if ((controlType != DISC) && (controlType != UA))
		readControlFrame(fd, controlType);

	char * controlFrame = malloc(CONTROL_FRAME_SIZE);

	if (read(fd, controlFrame, CONTROL_FRAME_SIZE) < CONTROL_FRAME_SIZE)
		return logError("Failed to read Control Frame");

	if ((controlFrame[FLAG1_POS] == FLAG) && (controlFrame[CF_POS] == controlType) && (controlFrame[AF_POS] == adressField) && 
		(controlFrame[FLAG2_POS] == FLAG) && (controlFrame[BCC_POS] == (controlFrame[AF_POS] ^ controlFrame[CF_POS])))
		return OK;


	free(controlFrame);
	return logError("Frame was note of the given type or Flags were not recognized\n");
}

int framingInformation(char* packet, uint* size) {

	int previousSize = (*size);
	(*size) += INF_FORMAT_SIZE;

	if ((packet = realloc(packet, (*size))) == NULL) {
		printf("framingInformation: Realloc error.\n");
		return ERROR;
	}

	//Setting the trailer
	packet[(*size) + TRAIL_BCC_POS] = 0; 
	/* TODO: BCC (Block Check Character) – detecção de erros baseada na geração de
	um octeto (BCC) tal que exista um número par de 1s em cada posição
	(bit), considerando todos os octetos protegidos pelo BCC (cabeçalho ou
	dados, conforme os casos) e o próprio BCC (antes de stuffing) */
	packet[(*size) + TRAIL_FLAG_POS] = FLAG;

	//Setting the Header
	memmove(packet + INF_HEAD_SIZE, packet, previousSize + INF_TRAILER_SIZE);
	packet[FLAG1_POS] = FLAG;
	packet[AF_POS] = AF1;
	packet[CF_POS] = (C_INF | (ll->seqNumber << 6));
	packet[BCC_POS] = (AF1 ^ packet[CF_POS]);

	return OK;
}

int deframingInformation(char* frame, uint* size) {
	//Checking the Header
	if ((frame[FLAG1_POS] != FLAG) || 
		(frame[AF_POS] != AF1) ||
		(frame[CF_POS] != (C_INF | (ll->seqNumber << 6))) ||		//TODO ll->receivedSeqNumber
		((frame[AF_POS] ^ frame[CF_POS]) != frame[BCC_POS]))
		logError("Received unexcpted head Information");

	//Checking the Trailer -> TODO change bcc accordingly to framingInformation
	int trailPos = (*size) - INF_TRAILER_SIZE; 
	if ((frame[trailPos + TRAIL_BCC_POS] != 0) || 
		(frame[trailPos + TRAIL_FLAG_POS]) != FLAG)
		logError("Received unexpected trailer Information");

	//Remove the framing
	(*size) -= INF_FORMAT_SIZE;

	memmove(frame, frame + INF_HEAD_SIZE, (*size));
	if ((frame = realloc(frame, (*size))) == NULL)
		return logError("Realloc error in deframingInformation");

	return OK;
} 

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
