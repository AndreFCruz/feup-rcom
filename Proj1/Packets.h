#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define SIZE2_MUL           256
#define HEADER_SIZE         4


//DATA_PACKET
#define CTRL_FIELD_IDX      0
#define SEQ_NUM_IDX         1
#define DATA_PACKET_SIZE2_IDX   2
#define DATA_PACKET_SIZE1_IDX   3

//CONTROL_PACKET
#define FILE_SIZE_ARG       0
#define FILE_NAME_ARG       1

typedef struct {
    uchar * data;
    uint size;
} Packet;

typedef enum {
    DATA = 0x01, START = 0x02, END = 0x03
} PacketType;

typedef struct {
    uchar seqNr;
    uint size;
    uchar * data;
} DataPacket;

typedef struct {
    PacketType type;
    char fileName[MAX_FILE_NAME];
    uint fileSize;
    uint argNr;
} ControlPacket;

void makeControlPacket(ControlPacket * src, Packet * dest);

void makeDataPacket(DataPacket * src, Packet * dest);

int sendDataPacket(DataPacket * src);

int sendControlPacket(ControlPacket * src);

int receiveDataPacket(DataPacket * dest);

int fillControlPacketArg(uchar * data, ControlPacket * dest, int argNr, int argSize, int offset);

int receiveControlPacket(ControlPacket * dest);