#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// TODO filtrar estes includes

#define FLAG 	0x7E
#define ESC 	0x7D
#define STUFFING 0X20
#define OK		0
#define ERROR	1
#define RESEND	2

#define CONTROL_FRAME_SIZE	5
#define INF_FORMAT_SIZE		6
#define INF_HEAD_SIZE		4
#define INF_TRAILER_SIZE 	2

//Information Message Trailers position
#define TRAIL_BCC_POS		0
#define TRAIL_FLAG_POS		1

//Positions of Fields in the Control Message
#define FLAG1_POS	1
#define AF_POS		2
#define CF 			3
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

//static struct linkLayer globalPtr;

typedef unsigned int uint;

enum ControlTypes {SET, DISC, UA, RR, REJ};

//TODO: REVER ISTO AQUI -> O R TEM QUE SER CONTROLADO DE ALGUMA FORMA
int nr = 0;
int ns = 0;

/**
 * Creates a Control Frame, according to the protocols.
 *
 * @param adressField The Adress field value.
 * @param controlField The Control field value.
 * @return The generated frame.
 */
char* createControlFrame(char adressField, char controlField) {
	char* buffer = malloc(CONTROL_FRAME_SIZE);

	buffer[FLAG1_POS] = FLAG;
	buffer[AF_POS] = adressField;
	buffer[CF_POS] = controlField;
	buffer[BCC_POS] = (adressField ^ controlField);
	buffer[FLAG2_POS] = FLAG;

	return buffer;
}

/**
 * Generates a Controll frame, corresponding to the given type. 
 *
 * @param type The type of control frame
 * @return The frame generated.
 */
char* genControlFrame(ControlTypes type) {
	int nrValue = (NR << 7);

	if (TRANSMITER) {
		switch(types) {
			CASE SET:
				return createControlFrame(AF1, C_SET);
			CASE DISC:
				return createControlFrame(AF1, C_DISC);
			CASE UA:
				return createControlFrame(AF2, C_UA);
			CASE RR:
				return createControlFrame(AF2, (C_RR | nrValue));
			CASE REJ:
				return createControlFrame(AF2, (C_REJ | nrValue));
		}
	}
	else if (RECEIVER) {
		switch(types) {
			CASE SET:
				return createControlFrame(AF2, C_SET);
			CASE DISC:
				return createControlFrame(AF2, C_DISC);
			CASE UA:
				return createControlFrame(AF1, C_UA);
			CASE RR:
				return createControlFrame(AF1, (C_RR & nrValue));
			CASE REJ:
				return createControlFrame(AF1, (C_REJ & nrValue));
		}
	}
	return NULL;
}

/**
 * Evaluates if the Frame's header is wrong, being descarted if so.
 *
 * @param frame The frame to evaluated
 * @param size The frame's size.
 * @return ERROR if something went wrong, OK otherwise
 */
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

	//Checking the Control Field
	if ((frame[CF_POS] != C_SET) ||
		(frame[CF_POS] != C_DISK) ||
		(frame[CF_POS] != C_UA) ||
		(frame[CF_POS] != (C_RR | NR)) ||
		(frame[CF_POS] != (C_RR | ~NR)) ||
		(frame[CF_POS] != (C_REJ| NR)) ||
		(frame[CF_POS] != (C_REJ| ~NR)) ||
		(frame[CF_POS] != (C_INF| NS)) ||
		(frame[CF_POS] != (C_INF| ~NS))) {		
		printf("Error in received Frame Header: Control Field\n");
		return ERROR;
	}

	//Checking the Protection Field
	if ((frame[AF_POS] ^ frame[CF_POS]) != frame[BCC_POS]) {
		printf("Error in received Frame Header: Protection Field\n");
		return ERROR;
	}

	return OK;
}

/**
 * Verifies if a Positive Acknoledgement was received.
 *
 *
 * @return OK if it was, ERROR otherwise.
 */
int receiveAck(int fd) {

}

/**
 * Creates a Information Frame, according to the protocols.
 * Final Frame and its size its retrivied in the function parameters.
 *
 * @param packet The packet to be framed.
 * @param size The packet's size.
 * @return ERROR if something went wrong, OK otherwise
 */
int createInfFrame(char* packet, uint* size) {

	int previousSize = (*size);
	(*size) += INF_FORMAT_SIZE;

	if ((packet = realloc(packet, (*size))) == NULL) {
		printf("createInfFrame: Realloc error.\n");
		return ERROR;
	}

	//Setting the trailer
	packet[(*size) + TRAIL_BCC_POS] = 0;
	/* TODO: BCC (Block Check Character) – detecção de erros baseada na geração de
um octeto (BCC) tal que exista um número par de 1s em cada posição
(bit), considerando todos os octetos protegidos pelo BCC (cabeçalho ou
dados, conforme os casos) e o próprio BCC (antes de stuffing) */
	packet[(*size) + TRAIL_FLAG_POS] = FLAG;

	memmove(packet + INF_HEAD_SIZE, packet, previousSize + INF_TRAILER_SIZE);
	packet[FLAG1_POS] = FLAG;
	packet[AF_POS] = AF1;
	packet[CF_POS] = (C_INF | (NS << 6));
	packet[BCC_POS] = (AF1 ^ packet[CF_POSF]);

	return OK;
}

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
int byteDestuffing(char* buffer, int* size) {
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

	do {
		if (createInfFrame(buffer, &length) == ERROR) {
			printf("llwrite error: Failed to create Information Frame.\n");
			return ERROR;
		}
		
		if (byteStuffing(buffer, &length) == ERROR) {
			printf("llwrite error: Failed to create Information Frame.\n");
			return ERROR;
		}

		if ((res = write(fd, buffer, length) < length)) {
			printf("llwrite error: Bad write: %d bytes\n", res);
			return ERROR;
		}
	} while (receiveAck(fd) == RESEND);

	return OK;
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
