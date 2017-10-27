#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "LinkLayer.h"
#include "Alarm.h"

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
#define FLAG1_POS			0
#define AF_POS				1
#define CF_POS 				2
#define BCC_POS 			3
#define FLAG2_POS			4

//Possible Adress Fields
#define AF1					0x03
#define AF2					0x01

#define MAX_PORT_NAME 		16
#define PORT_NAME			"/dev/ttyS"
// #define PORT_NAME			"/dev/ttys" // for MacOS


typedef enum {
	INF = 0x00, SET = 0x03, DISC = 0x0B, UA = 0x07, RR = 0x05, REJ = 0x01
} ControlType;

typedef struct {
	char port[MAX_PORT_NAME];
	int baudRate;
	uint seqNumber;
	uint timeout;
	uint numRetries;
} LinkLayer;

static LinkLayer * ll = NULL;

static struct termios oldtio;

static ConnectionType connectionType;


int readFromSerialPort(int fd, uchar ** dest);

/**
 * Keeps reading until a FRAME FLAG is found.
 *
 * @param fd The Serial Port's filedescriptor
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
void createControlFrame(uchar buffer[], uchar adressField, uchar controlField);

/**
 * Sends a Control frame of the given type.
 *
 * @param fd The Serial Port filedescriptor
 * @param controlType The type of control frame
 * @return Number of written bytes, -1 if an error happened.
 */
int sendControlFrame(int fd, ControlType controlType);

/**
 * Reads a frame and checks if it is of the given type.
 *
 * @param fd The Serial Port's filedescriptor
 * @param controlType The type of control frame
 * @return OK if the frame was of the given type, ERROR otherwise.
 */
int readControlFrame(int fd, ControlType controlType);

/**
 * The Data Field BCC calculator
 *
 * @param buffer The adress containing the first data byte
 * @param length The data field length
 * @return The calculated bcc
 */
uchar calcBCC(uchar * buffer, size_t length);

/**
 * Creates a Information Frame, according to the protocols.
 * Final Frame and its size its retrivied in the function parameters.
 *
 * @param packet The packet to be framed.
 * @param size The packet's size.
 * @return ERROR if something went wrong, OK otherwise
 */
int framingInformation(uchar ** packet, int* size);

/**
 * Evaluates if the framing is wrong, being descarted if so.
 * The deframed frame and its size are returned in the given arguments.
 *
 * @param frame The frame to be evaluated
 * @param size The frame's size.
 * @return ERROR if something went wrong, OK otherwise
 */
int deframingInformation(uchar ** frame, int* size);

/**
 * Applies byte stuffing to the given message according to the protocols, retriving the new message in the same buffer.
 *
 * @param buffer The buffer containing the message to be stuffed.
 * @param size The buffer's size
 * @return ERROR if something went wrong, OK otherwise
 */
int byteStuffing(uchar ** buffer, int * size);

/**
 * Applies byte destuffing to the given message according to the protocols, retriving the new message in the same buffer.
 *
 * @param buffer The buffer containing the stuffed message.
 * @param size The buffer's size
 * @return Error if something went wrong, ok otherwise
 */
int byteDestuffing(uchar * buffer, int * size);



int initLinkLayer(int porta, int baudRate, uint timeout, uint nRetries) {
	if (ll == NULL)
		ll = malloc(sizeof(LinkLayer));
	else return logError("LinkLayer already initialized");

	snprintf(ll->port, MAX_PORT_NAME, "%s%d", PORT_NAME, porta);

	ll->baudRate = baudRate;
	ll->timeout = timeout;
	ll->numRetries = nRetries;
	ll->seqNumber = 0;

	return OK;
}

int openSerialPort() {
	if (ll == NULL)
		return logError("LinkLayer not initialized");

	int fd = open(ll->port, O_RDWR | O_NOCTTY );
	if (fd < 0) { perror(ll->port); exit(-1); }

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

	newtio.c_cc[VTIME]    = ll->timeout * 10;   /* inter-ucharacter timer unused - in 0.1s*/
	newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 uchars received */
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
	connectionType = type;

	int fd = openSerialPort();

	switch (type) {
	case TRANSMITTER:
		if ((sendControlFrame(fd, SET) > 0) && (readControlFrame(fd, UA) == OK))
			return fd;
		break;
	case RECEIVER:
		if ((readControlFrame(fd, SET) == OK) && (sendControlFrame(fd, UA) > 0))
			return fd;
		break;
	}

	printf("Failed llopen\n");
	return -1;
}

void llcloseTransmitter(int fd) {
	sendControlFrame(fd, DISC);
	readControlFrame(fd, DISC);
	sendControlFrame(fd, UA);
}

void llcloseReceiver(int fd) {
	readControlFrame(fd, DISC);
	sendControlFrame(fd, DISC);
	readControlFrame(fd, UA);
}

int llclose(int fd) {
	printf("** llclose called **");

	if (TRANSMITTER == connectionType)
		llcloseTransmitter(fd);
	else if (RECEIVER == connectionType)
		llcloseReceiver(fd);
	else
		logError("llclose: no connection type set");

	//Reset terminal to previous configuration
	if ( tcsetattr(fd,TCSANOW,&oldtio) == -1 ) {
		perror("tcsetattr");
		return -1;
	}

	if (close(fd) < 0) {
		perror("failed close(fd)");
		return -1;
	}

	return OK;
}

int llwrite(int fd, uchar ** bufferPtr, int length) {
	int res = 0;

	printf("**llwrite called**");

	if (framingInformation(bufferPtr, &length) == ERROR) {
		printf("llwrite error: Failed to create Information Frame.\n");
		return -1;
	}

	if (byteStuffing(bufferPtr, &length) == ERROR) {
		printf("llwrite error: Failed to create Information Frame.\n");
		return -1;
	}

	uint tries = 0;
	setAlarm();
	do {
		alarmWentOff = FALSE;
		printf("llwrite: ");
		if ((res = write(fd, *bufferPtr, length)) < length) {
			printf("llwrite error: Bad write: %d bytes\n", res);
			return -1;
		}
		printf(" tentativa %d.\n", tries);

		alarm(ll->timeout);
	} while ( (readControlFrame(fd, RR) != OK) && (++tries < (ll->numRetries)));

	stopAlarm();

	if (tries >= ll->numRetries)
		return ERROR;
	//TODO: Fazer alguma coisa quando as tentativas ultrapassarem? é que nao esta a afazer nada,
	//dai de certo reiniciar... Ver o que acontece ao reader. Fica infinitamente a espera?

	return res;
}


int readFromSerialPort(int fd, uchar ** dest) {
	uchar * buffer = (uchar *) malloc(RECEIVER_SIZE);
	int bufferIdx = 0;		//Number of bytes received
	int res;

	if (readFrameFlag(fd) != OK) {
		printf("readFromSerialPort Error: read Frame flag error\n");
		return -1;
	}

	printf("\nStarting reading loop\n");
	buffer[bufferIdx++] = FLAG;
	do {
		if (alarmWentOff == TRUE)
			return -1;

		res = read(fd, buffer + bufferIdx, sizeof(uchar));
		if ( res < 0 ) {
			printf("readFromSerialPort error: Failed to read from SerialPort\n");
			return -1;
		} else if (res == 0) {
			continue;
		}
		printf(".");
		++bufferIdx;
		if ( ((bufferIdx + 1) % RECEIVER_SIZE) == 0 ) {
			printf("incremented buffer size from %d to %d\n", bufferIdx, ((bufferIdx + 1) / RECEIVER_SIZE + 1) * RECEIVER_SIZE );
			if ((buffer = realloc(buffer,  ((bufferIdx + 1) / RECEIVER_SIZE + 1) * RECEIVER_SIZE)) == NULL) {
				printf("readFromSerialPort error: Failed to realloc buffer\n");
				return -1;
			}
		}
	} while (buffer[bufferIdx - 1] != FLAG);

	*dest = buffer;

	return bufferIdx;
}

int llread(int fd, uchar ** dest) {
	uint tries = 0;
	int ret;
	while ( 1 /* tries++ < ll->numRetries */ ){
		if ( (ret = readFromSerialPort(fd, dest)) > 0 ) {
			if (byteDestuffing(*dest, &ret) == ERROR) {
				logError("llread error: Failed byteDestuffing");
				free(*dest);
				continue;
			}

			if (deframingInformation(dest, &ret) != OK) {
				// enviar REJ aqui para antecipar TIMEOUT ?
				logError("llread Error: Failed to deframe information");
				free(*dest);
				continue;
			}

			sendControlFrame(fd, RR);
			return ret;
		}
	}

	return -1;
}

int readFrameFlag(int fd) {
	uchar tempchar = 0;
	int res;

	// Loop para ler 1 caracter: se for a FLAG OK;
	// se nao, passar para o loop seguinte e descartar tudo até
	// ler uma flag (incluindo a flag) e dar return ERROR
	while (alarmWentOff == FALSE) {
		res = read(fd, &tempchar, 1);
		printf("\t** ReadFrameFlag: %02X . NumBytes: %d\n", tempchar, res);

		if (res == 0)
			continue;
		else if (res == 1 && tempchar == FLAG)
			return OK;
		else
			break;
	}

	while (tempchar != FLAG && alarmWentOff == FALSE) {
		printf(" ** Received Garbage : %02X **\n", tempchar);
		read(fd, &tempchar, sizeof(uchar));
	}
	printf("\t** Exited garbage eater\n");

	return ERROR;
}

void createControlFrame(uchar buffer[], uchar adressField, uchar controlField) {
	buffer[FLAG1_POS] = FLAG;
	buffer[AF_POS] = adressField;
	buffer[CF_POS] = controlField;
	buffer[BCC_POS] = (adressField ^ controlField);
	buffer[FLAG2_POS] = FLAG;
}

int sendControlFrame(int fd, ControlType controlType) {
	uchar controlFrame[CONTROL_FRAME_SIZE];
	uint nrValue = (ll->seqNumber << 7);
	uchar afValue = AF1;

	if (((connectionType == RECEIVER) && (controlType == DISC)) ||
		((connectionType == TRANSMITTER) && (controlType == UA)))
		afValue = AF2;

	if ((controlType == RR) || (controlType ==REJ)) {
		createControlFrame(controlFrame, afValue, (controlType | nrValue));
	} else {
		createControlFrame(controlFrame, afValue, controlType);
	}

	int res;
	if ((res = write(fd, controlFrame, CONTROL_FRAME_SIZE)) < CONTROL_FRAME_SIZE) {
		printf("senControlFrame error: Bad write: %d bytes\n", res);
		return -1;
	}

	printf("Send control frame: ");
	printArray(controlFrame, CONTROL_FRAME_SIZE);

	return res;
}

// TODO Dividir em duas funcoes, uma para ler uma control frame, outra para confirmar o tipo
int readControlFrame(int fd, ControlType controlType) {
	uchar * controlFrame;
	uchar afValue;

	switch (controlType) {
	// Comandos
	case SET:
	case INF:
	case DISC:
		afValue = connectionType == TRANSMITTER ? AF2 : AF1;
		break;
	// Respostas
	case UA:
	case REJ:
	case RR:
		afValue = connectionType == TRANSMITTER ? AF1 : AF2;
		break;
	default:
		return logError("Bad serial port");
	}

	int res;
	if ((res = readFromSerialPort(fd, &controlFrame)) < CONTROL_FRAME_SIZE) {
		printf("Failed to read control frame. Read %d. Ctrl type: %02X -- ", res, controlType);
		printArray(controlFrame, res);
		return ERROR;
	}

	printf("Read control frame: ");
	printArray(controlFrame, CONTROL_FRAME_SIZE);
	printf("%02X, %02X, %02X, %02X, %02X\n", FLAG, afValue, controlType, (controlFrame[AF_POS] ^ controlFrame[CF_POS]), FLAG);

	if ((controlFrame[FLAG1_POS] == FLAG) &&
		((controlFrame[CF_POS] & 0x7F) == controlType) &&
		(controlFrame[AF_POS] == afValue) &&
		(controlFrame[FLAG2_POS] == FLAG) &&
		(controlFrame[BCC_POS] == (controlFrame[AF_POS] ^ controlFrame[CF_POS])))
	{
		uchar seqNrToReceive =  (~(ll->seqNumber)) << 7;
		printf("Current seqNr: %02X. Received seqNr: %02X. Modified seqNr: %02x\n", ll->seqNumber, controlFrame[CF_POS] & 0xA0, seqNrToReceive);
		if ((controlType == RR) || (controlType == REJ)) {
			if (controlFrame[CF_POS] == (controlType | seqNrToReceive)) {
				ll->seqNumber = ll->seqNumber ? 0 : 1;
				return OK;
			}
			return logError("Sequence number not aligned");
		}
	} else {
		return logError("Frame was not of the given type or Flags were not recognized");
	}

	free (controlFrame);

	return OK;
}

uchar calcBCC(uchar * buffer, size_t length) {
	uint i;
	uchar bcc = 0;
	for (i = 0; i < length; ++i)
		bcc ^= buffer[i];

	return bcc;
}

int framingInformation(uchar ** packet, int* size) {

	uint previousSize = (*size);
	(*size) += INF_FORMAT_SIZE;

	if ((*packet = realloc(*packet, (*size))) == NULL) {
		printf("framingInformation: Realloc error.\n");
		return ERROR;
	}

	(*packet)[previousSize + TRAIL_BCC_POS] = calcBCC((*packet), previousSize);
	(*packet)[previousSize + TRAIL_FLAG_POS] = FLAG;

	//Setting the Header
	memmove((*packet) + INF_HEAD_SIZE, *packet, previousSize + INF_TRAILER_SIZE);
	(*packet)[FLAG1_POS] = FLAG;
	(*packet)[AF_POS] = AF1;
	(*packet)[CF_POS] = (INF | (ll->seqNumber << 6));
	(*packet)[BCC_POS] = (AF1 ^ (*packet)[CF_POS]);

	printf("\t\t** Made frame with seqNr: %d\n", ll->seqNumber);

	return OK;
}

int deframingInformation(uchar ** frame, int* size) {
	//Checking the Header
	int flagPred = ((*frame)[FLAG1_POS] != FLAG);
	int afPred = ((*frame)[AF_POS] != AF1);
	int seqNrPred = ((*frame)[CF_POS] != (INF | (ll->seqNumber << 6)));
	int bccPred = (((*frame)[AF_POS] ^ (*frame)[CF_POS]) != (*frame)[BCC_POS]);

	if ( flagPred || afPred || bccPred ) {
		printf("Unexpected info on deframe: ");
		printArray((*frame), *size);
		printf("Flag: %d. AF: %d. SeqNr: %d. BCC: %d.\n", flagPred, afPred, seqNrPred, bccPred);
		return ERROR;
	}

	//Checking the Trailer
	uint trailPos = (*size) - INF_TRAILER_SIZE;
	uchar bcc = calcBCC((*frame) + INF_HEAD_SIZE, trailPos - INF_HEAD_SIZE);

	if ((*frame)[trailPos + TRAIL_BCC_POS] != bcc) {
		//sendControlFrame(REJ);	//O que faz ele na receção de um BCC? ver protocolo
		return logError("received unexpected Data Field BCC2\n");
	}
	if ((*frame)[trailPos + TRAIL_FLAG_POS] != FLAG)
		return logError("Received unexpected value instead of trailer FLAG\n");

	printf("** DEFRAME SeqNr: %d .. ", ll->seqNumber);
	//read's Nr is negative of sender's Ns
	// THIS MUST BE AFTER CHECKING FRAME INFO
	if (!seqNrPred) {
		ll->seqNumber = ((*frame)[CF_POS] >> 6) & 0b01 ? 0 : 1;
	}
	printf("** NEW seqNr: %d \n", ll->seqNumber);

	//Remove the framing
	(*size) -= INF_FORMAT_SIZE;

	memmove((*frame), (*frame) + INF_HEAD_SIZE, (*size));
	if (((*frame) = realloc(*frame, (*size))) == NULL)
		return logError("Realloc error in deframingInformation");

	return (!seqNrPred);
}

int byteStuffing(uchar ** buffer, int * size) {
	uint i, cnt = 0;

	// Number of bytes needed to be stuffed
	for (i = BCC_POS; i < (*size) - 1; ++i) {
		if (((*buffer)[i] == (uchar) FLAG) || ((*buffer)[i] == (uchar) ESC))
			++cnt;
	}
	if (cnt == 0) return OK;

	*size = (*size) + cnt;

	if (((*buffer) = realloc((*buffer), *size)) == NULL) { // may abort
		return logError("ByteStuffing: Realloc error.");
	}

	for (i = BCC_POS; i < (*size) - 1; ++i) {
		if (((*buffer)[i] == (uchar) FLAG) || ((*buffer)[i] == (uchar) ESC))
		{
			memmove((*buffer) + i + 1, (*buffer) + i, (*size) - i);
			(*buffer)[i] = ESC;
			(*buffer)[i+1] = ((*buffer)[i+1] ^ STUFFING);
			i++;
		}
	}

	return OK;
}

int byteDestuffing(uchar * buffer, int * size) {
	uint i;

	for (i = BCC_POS; i < (*size) - 1; ++i) {
		if (buffer[i] == (uchar) ESC)
		{
			memmove(buffer+i, buffer+i+1, (*size)-i);
			--(*size);

			buffer[i] = (buffer[i] ^ STUFFING);
		}
	}

	// TODO realloc?

	return OK;
}
