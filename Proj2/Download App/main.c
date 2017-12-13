#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>

#include "getip.c"
//#include "clientTCP.h"

#define SERVER_PORT 21
#define SERVER_ADDR "192.168.28.96"
#define BUFFER_SIZE 1024

char currIp[32];

void printArray(char buffer[], int size) {
	int i;
	for (i = 0; i < size; i++) {
		printf("%c", buffer[i]);
	}
	printf("\n");
}

int connectSocket(const char* ip, int port) {
	int	sockfd;
	struct	sockaddr_in server_addr;

	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(port);		/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
				perror("socket()");
					exit(0);
			}
	/*connect to the server*/
			if(connect(sockfd,
						 (struct sockaddr *)&server_addr,
			 sizeof(server_addr)) < 0){
					perror("connect()");
		return -1;
	}

	return sockfd;
}


void printUsage(char* argv0) {
	printf("\nUsage1 Normal: %s ftp://[<user>:<password>@]<host>/<url-path>\n",
			argv0);
	printf("Usage2 Anonymous: %s ftp://<host>/<url-path>\n\n", argv0);
}


int main(int argc, char** argv){
	if (argc != 2) {
			printf("WARNING: Wrong number of arguments.\n");
			printUsage(argv[0]);
			exit(1);
    }


  strcpy(currIp, getIp(argv[1]));

	int	sockfd;
	if((sockfd = connectSocket(currIp, SERVER_PORT)) == -1)
		exit(1);

	char buf[BUFFER_SIZE];
	int	bytes;

    	/*send a string to the server*/
	while((bytes = recv(sockfd, buf, BUFFER_SIZE,0)) > 0){
		printf("Bytes Read %d\n",bytes);
		printArray(buf,bytes);
	}

	close(sockfd);
	exit(0);
}
