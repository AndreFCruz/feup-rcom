enum ConnectionType {
  TRANSMITTER, RECEIVER
};

typedef struct {
  int fileDescriptor; // serial port's file descriptor
  ConnectionType status;
} ApplicationLayer;

typedef struct {
  unsigned char * array;
  unsigned int size;
} Packet;

int sendDataPacket(Packet * packet);
int sendCmdPacket(Packet * packet);
