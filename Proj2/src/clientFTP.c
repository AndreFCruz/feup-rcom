#include "clientFTP.h"
#include "URL.h"
#include "utils.h"

static FTP * ftp;
static URL * url;

static void quitConnection();

static void forceQuit(char * errorMSg){
	int errorStatus = logError(errorMSg);
	quitConnection();
	exit(errorStatus);
}

static void printResponse(char * responseCmd){
  	if(DEBUG)
    	printf("%s", responseCmd);
  	else {
    	int msgLength = strlen(responseCmd)-RESPONSE_CODE_OFFSET;
   		char * msgWithoutCode = (char *) malloc(msgLength);
    	memcpy(msgWithoutCode, responseCmd+RESPONSE_CODE_OFFSET, msgLength+1);
    	printf("%s", msgWithoutCode);
    	free(msgWithoutCode);
  	}
}

static int receiveCommand(int fd, char* responseCmd, char * expectedAnswer) {

	FILE* fp = fdopen(fd, "r");
	int allocated = 0;
	if(responseCmd == NULL) {
		responseCmd = (char*) malloc(sizeof(char) * MESSAGE_SIZE);
		allocated = 1;
	}
	do {
		memset(responseCmd, 0, MESSAGE_SIZE);
		responseCmd = fgets(responseCmd, MESSAGE_SIZE, fp);
		printResponse(responseCmd);
	}  while (!('1' <= responseCmd[0] && responseCmd[0] <= '5') || responseCmd[3] != ' ');

	char responseStatus = responseCmd[0];

	if(expectedAnswer != NULL && strncmp(expectedAnswer, responseCmd, strlen(expectedAnswer))){
		if(allocated)
			free(responseCmd);
		return ERROR;
	}

	if(allocated)
		free(responseCmd);

	return (responseStatus > '4');
}

static int sendCommand(int fd, const char* msg, char* response, int readResponse, char * expectedAnswer) {
	int nBytes = write(fd, msg, strlen(msg));
	if (readResponse) 
		return receiveCommand(fd, response, expectedAnswer);
	else return (nBytes == 0);
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
	if (connect(sockfd, (struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
			perror("connect()");
			return -1;
	}

	return sockfd;
}

static void retrieveFile(int fd) {

	char userCommand[MESSAGE_SIZE + 1];

	sendCommand(fd, "TYPE L 8\r\n", NULL, TRUE, SET_TYPE); //Setting type of file to be transferred -> local file
	sprintf(userCommand, "RETR %s%s\r\n", url->path, url->filename);
	if(sendCommand(fd, userCommand, NULL, TRUE, NULL) != OK)
		forceQuit("Failed to retrieve file. Terminating Program.");
}

static void sendUSER(int fd) {

	char userCommand[MESSAGE_SIZE + 1];
	char passCommand[MESSAGE_SIZE + 1];

	receiveCommand(fd, NULL, NULL);

	if (strcmp(url->username, "anonymous") == OK)
		printf("Logging in: anonymous mode.\n");
	else
		printf("Logging in: authentication mode");

	sprintf(userCommand, "USER %s\r\n", url->username);
	if(sendCommand(fd, userCommand, NULL, TRUE, REQUIRED_PASSWORD) != OK)
		forceQuit("Failed to log in, wrong username?\nTerminating Program.");


	sprintf(passCommand, "PASS %s\r\n", url->password);
	if(sendCommand(fd, passCommand, NULL, TRUE, SUCCESS_LOGIN) != OK)
		forceQuit("Failed to log in, wrong password?\nTerminating Program.");
}

static void sendPASV(int fd) {

	char response[MESSAGE_SIZE + 1];

	if(sendCommand(fd, "PASV\r\n", response, TRUE, PASSIVE_MODE) != OK)
		forceQuit("Failed to enter passive mode. Terminating program.");


	int remoteIP[6];
	char* data = strchr(response, '(');
	sscanf(data, "(%d, %d, %d, %d, %d, %d)", &remoteIP[0],&remoteIP[1],&remoteIP[2],&remoteIP[3],&remoteIP[4],&remoteIP[5]);
	sprintf(url->ip, "%d.%d.%d.%d", remoteIP[0],remoteIP[1],remoteIP[2],remoteIP[3]);
	url->port = remoteIP[4]*256+remoteIP[5];
}

static int download(int fd) {
	FILE* outfile;
	if( !(outfile = fopen(url->filename, "w")) )
		return logError("Unable to open file.");

	int dots = 0;
	printf("Downloading.");

	char buf[SOCKET_SIZE];
	int bytes;
	while ( (bytes = read(fd, buf, sizeof(buf))) != 0 ) {
		if (bytes < 0)
			return logError("Empty data socket, nothing to receive.\n");

		if ((bytes = fwrite(buf, bytes, 1, outfile)) < 0) {
			return logError("Unable to write data in file.\n");
		}

		printDownloadProgress(&dots);
	}

	fclose(outfile);

	printf("\rDownload finished with success.\n");

	return OK;
}

static void quitConnection() {

	printf("Closing connection with the server.\n");
	if(sendCommand(ftp->fdControl, "QUIT\r\n", NULL, FALSE, NULL) != OK) {
		close(ftp->fdData);
		close(ftp->fdControl);
		exit(logError("Failed to exit connection. Forcing exit.\n"));
	}

	close(ftp->fdData);
	close(ftp->fdControl);
}

int downloadFtpUrl(const char * str) {

	ftp = (FTP *) malloc(sizeof(FTP));

	url = constructURL();
	if (parseURL(url, str))
		exit(logError("Error parsing url. Please provide a valid url.\n"));
		
	setIp(url);

	printURL(url);

	if((ftp->fdControl = connectSocket(url->ip, url->port)) == 0)
		exit(logError("Failed to open control conection. Terminating Program.\n"));

	sendUSER(ftp->fdControl);
	sendPASV(ftp->fdControl);

	if((ftp->fdData = connectSocket(url->ip, url->port)) == 0)
		exit(logError("Failed to open data connection. Terminating Program.\n"));

	retrieveFile(ftp->fdControl);
	download(ftp->fdData);

	destructURL(url);
	quitConnection();
	free(ftp);

	printf("TERMINATING PROGRAM\n");

	return OK;
}