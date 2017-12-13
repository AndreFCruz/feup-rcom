#include "clientTCP.h"

FTP * ftp;

static char* receiveCommand(int fd, const char* responseCmd) {

	char* responseMessage = (char*) malloc(MESSAGE_SIZE + 1);

	if (read(fd, responseMessage, MESSAGE_SIZE) <= 0) {
		return NULL;
	}

	if (responseCmd != NULL && strncmp(responseCmd, responseMessage, strlen(responseCmd))) {
		printf("[ERROR] %s", responseMessage);
		return NULL;
	}

	return responseMessage;
}

static int sendCommand(int fd, const char* msg, unsigned length) {

	int nBytes = write(fd, msg, length);

	if (nBytes <= 0) {
		return FALSE;
	}

	LOG_FORMAT("sent message: %s", msg);

	return TRUE;
}

static int connectSocket(const char* ip, int port) {
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

int startConnection(char* serverUrl) {

	if (!parseURL(serverUrl)) {
		return FALSE;
	}

	ftp->fdControl = connectSocket(url->serverIP, url->serverPort);

	if (ftp->fdControl < 0) {
		perror("socket()");
		return FALSE;
	}

	// PRINT HOST AND CONNECTION INFORMATION
	debugFTP();

	if (!receiveCommand(ftp->fdControl, SERVICE_READY)) {
		ERROR("received invalid response from server, expected [220:SERVICE_READY]");
	}

	if (!sendUSER(ftp->fdControl)) {
		return FALSE;
	}

	if (!sendPASV(ftp->fdControl)) {
		return FALSE;
	}

	int userInput;

	if (url->serverFile == NULL) {
		action_listDirectory();
	}
	else {
		puts("\n> PLEASE CHOOSE AN OPTION:");
		puts("(1) List Directory\n(2) Download File\n(0) Exit Application\n");
		userInput = readInteger(0, 2);

		if (userInput == 1) {
			action_listDirectory();
		}
		else if (userInput == 2) {
			action_retrieveFile();
		}
		else if (userInput == 0) {
			return TRUE;
		}
	}

	return action_quitConnection();
}

int action_quitConnection(void) {

	// SEND "QUIT" COMMAND
	if (!sendCommand(ftp->fdControl, "QUIT\r\n", strlen("QUIT\r\n"))) {
		ERROR("sending QUIT command to server failed!");
	}

	if (ftp->fdData && close(ftp->fdData) < 0) {
		ERROR("%s connection problem: attempt to disconnect failed.\n");
	}

	if (ftp->fdControl && close(ftp->fdControl) < 0) {
		ERROR("%s connection problem: attempt to disconnect failed.\n");
	}

	return TRUE;
}
