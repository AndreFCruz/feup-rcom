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
#include <sys/stat.h>
#include <fcntl.h>

#define DEBUG					0	

#define SERVER_READY      	"220"
#define TRANSFER_COMPLETE 	"226"
#define REQUIRED_PASSWORD	"331"
#define SUCCESS_LOGIN     	"230"
#define FINISHED	      	"150"
#define	PASSIVE_MODE	  	"227"
#define DIRECTORY_OK	  	"250"
#define SET_TYPE  			"200"

typedef struct ftp_t {
	int fdControl;
	int fdData;
} FTP;

int downloadFtpUrl(const char * str);

#endif /* __CLIENTFTP_H */