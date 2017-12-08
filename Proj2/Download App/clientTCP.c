/*      (C)2000 FEUP  */

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

int main(int argc, char** argv){


	if (argc != 2) {  
          fprintf(stderr,"usage: getip address\n");
          exit(1);
    }


    strcpy(currIp, getIp(argv[1]));

	int	sockfd;
	struct	sockaddr_in server_addr;
	char	buf[BUFFER_SIZE];
	int	bytes;
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(currIp);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
    
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
		exit(0);
	}
    	/*send a string to the server*/
	while((bytes = recv(sockfd, buf, BUFFER_SIZE,0)) > 0){
		printf("Bytes Read %d\n",bytes);
		printArray(buf,bytes);
	}

	close(sockfd);
	exit(0);
}


