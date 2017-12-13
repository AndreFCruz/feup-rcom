#ifndef __CLIENTFTP_H
#define __CLIENTFTP_H

#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <string.h>

typedef struct ftp_t {
	char* userName;
	char* userPassword;
	int fdControl;
	int fdData;
} FTP;

int startConnection(char* serverUrl);

#endif /* __CLIENTFTP_H */
