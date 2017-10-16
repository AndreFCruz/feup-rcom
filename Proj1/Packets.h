#include "utils.h"

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

// ??
DataPacket makeControlPacket(ControlPacket * src);
