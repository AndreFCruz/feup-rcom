#include "clientFTP.h"
#include "URL.h"
#include "utils.h"

FTP * ftp;
URL * url;

static int receiveCommand(int fd, char* responseCmd) {

	/*char* responseMessage = (char*) malloc(MESSAGE_SIZE + 1);

	printf("Response: %s\n", responseMessage);

	if (read(fd, responseMessage, MESSAGE_SIZE) <= 0) {
		return NULL;
	}

	if (responseCmd != NULL && strncmp(responseCmd, responseMessage, strlen(responseCmd))) {
		//printf("ERROR: %s", responseMessage);
		logError(responseMessage);   //TODO
		return NULL;
	}

	return responseMessage;*/






	FILE* fp = fdopen(fd, "r");
  int allocated = 0;
  if(responseCmd == NULL) {
    responseCmd = (char*) malloc(sizeof(char) * MESSAGE_SIZE);
    allocated = 1;
  }
  do {
    memset(responseCmd, 0, MESSAGE_SIZE);
    responseCmd = fgets(responseCmd, MESSAGE_SIZE, fp);
    printf("%s", responseCmd);
	}  while (!('1' <= responseCmd[0] && responseCmd[0] <= '5') || responseCmd[3] != ' ');
  char reply_series = responseCmd[0];
  if(allocated)
    free(responseCmd);
  return (reply_series > '4');
}

static int sendCommand(int fd, const char* msg, char* response, unsigned readAnswer) {

	/*int nBytes = write(fd, msg, length);

	if (nBytes <= 0) {
		return FALSE;
	}

	//printf("sent message: %s", msg);

	return TRUE;*/

	int nBytes = write(fd, msg, strlen(msg));
	if (readAnswer)
		return receiveCommand(fd, response);
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
	if(connect(sockfd, (struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
			perror("connect()");
			return -1;
	}

	return sockfd;
}

static void retrieveFile(int fd) {

	char userCommand[MESSAGE_SIZE + 1];

	sendCommand(fd, "TYPE L 8\r\n", NULL, 1); //Setting type of file to be transferred -> local file
	sprintf(userCommand, "RETR %s%s\r\n", url->path, url->filename);
  if(sendCommand(fd, userCommand, NULL, 1) != 0){
    printf("Error retrieving file. Exiting...\n");
    exit(1);
  }
}
/*
	// SWITCH TO CURRENT WORKING DIRECTORY
	if (url->path != NULL && !sendCWD()) {
		return FALSE;
	}

	// FORMAT "RETR" COMMAND ARGUMENTS
	sprintf(userCommand, "RETR %s\r\n", url->filename);

	// SEND "RETR" COMMAND
	if (!sendCommand(ftp->fdControl, userCommand, strlen(userCommand))) {
		return logError("sending RETR command to server failed!");
	}

	// CHECK IF COMMAND RETURN CODE IS VALID
	char* responseCommand = receiveCommand(ftp->fdControl, FINISHED);
	if (!responseCommand) {
		return logError("received invalid response from server, file not found?");
	}

	char expectedFilename[PATH_MAX];
	int unknownSize = FALSE;
	unsigned fileSize = 0;

	if (sscanf(responseCommand, "150 Opening BINARY mode data connection for %s (%d bytes)", expectedFilename, &fileSize) < 2) {
		unknownSize = TRUE;
	}

	printf("[INFORMATION] created output file: %s\n", url->filename);

	if (!unknownSize) {
		printf("[INFORMATION] output file size: %d (bytes)\n", fileSize);
	}

	printf("[INFORMATION] starting file transfer...");

	int fd = open(url->filename, O_WRONLY | O_CREAT, 0600);

	if (fd < 0) {
		perror(url->filename);
		return FALSE;
	}

	/*
	char dataBuffer[SOCKET_SIZE];
	int length;
	int bytesRead = 0;
	int bytesWritten = 0;
	int bytesSinceUpdate = 0;
	long long lastUpdate = getCurrentTime();
	long long totalTime = 0LL;
	long long currentUpdate = 0LL;
	double transferSpeed = 0.0;

	while ((length = read(ftp->fdData, dataBuffer, SOCKET_SIZE)) > 0) {

		if ((bytesWritten = write(fd, dataBuffer, length)) < 0) {
			return logError("could not write data to output file!");
		}

		bytesRead += length;
		bytesSinceUpdate += length;
		currentUpdate = getCurrentTime();

		if (currentUpdate - lastUpdate > 400) {
			transferSpeed = bytesSinceUpdate / (double) (currentUpdate - lastUpdate);
			logProgress(bytesRead, fileSize, transferSpeed);
			bytesSinceUpdate = 0;
			totalTime += currentUpdate - lastUpdate;
			lastUpdate = currentUpdate;
		}
	}
	*/
/*
	char dataBuffer[SOCKET_SIZE];
	int length;
	int bytesWritten = 0;

	while ((length = read(ftp->fdData, dataBuffer, SOCKET_SIZE)) > 0) {
		if ((bytesWritten = write(fd, dataBuffer, length)) < 0) {
			return logError("could not write data to output file!");
		}
	}

	if (close(fd) < 0) {
		perror(url->filename);
		return FALSE;
	}

	// CHECK IF COMMAND RETURN CODE IS VALID
	if (!receiveCommand(ftp->fdControl, TRANSFER_COMPLETE)) {
		return logError("received invalid response from server, connection interrupted?");
	}

	/*
	// CHECK EXPECTED FILE SIZE
	if (!unknownSize) {

		if (fileSize != bytesRead) {
			return logError("expected and received file sizes don't match!");
		}

		transferSpeed = bytesSinceUpdate / (double) (currentUpdate - lastUpdate);
		logProgress(bytesRead, fileSize, transferSpeed);
	}
	*/
/*
	printf("[INFORMATION] file transfer completed successfully!");
	//printf("[INFORMATION] TOTAL BYTES RECEIVED: %d bytes\n", bytesRead);
	//printf("[INFORMATION] AVERAGE TRANSFER SPEED: %.2f kBytes/sec\n", (double) bytesRead / totalTime);

	return TRUE;
}*/

static void sendUSER(int fd) {

	char userCommand[MESSAGE_SIZE + 1];
	char passCommand[MESSAGE_SIZE + 1];

	receiveCommand(fd, NULL);

	sprintf(userCommand, "USER %s\r\n", url->user);
	sendCommand(fd, userCommand, NULL, 1);
	sprintf(passCommand, "PASS %s\r\n", url->password);
  if(sendCommand(fd, userCommand, NULL, 1) != 0) {
      printf("Bad login. Exiting...\n"); //TODO: Ask for valid login
      exit(1);
	}
}
/*
	// FORMAT "USER" COMMAND ARGUMENTS
	if (url->user == NULL || strcmp("anonymous", url->user) == 0) {
		printf("[INFORMATION] entering anonymous mode...");
		sprintf(userCommand, "USER %s\r\n", "anonymous");
		anonymousMode = TRUE;
	}
	else {
		puts("[INFORMATION] entering authentication mode...");
		sprintf(userCommand, "USER %s\r\n", url->user);
	}

	if (url->password == NULL && !anonymousMode) {
		return logError("user must enter a password in authentication mode!");
	}

	// FORMAT "PASS" COMMAND ARGUMENTS
	sprintf(passCommand, "PASS %s\r\n", url->password);

	// SEND "USER" COMMAND
	if (!sendCommand(ftp->fdControl, userCommand, strlen(userCommand))) {
		return logError("sending USER command to server failed!");
	}

	// CHECK IF COMMAND RETURN CODE IS VALID
	char* responseCommand = receiveCommand(ftp->fdControl, NULL);
	//printf("Response: %s\n", responseCommand);
	int isAskingPassword = (strncmp(REQUIRED_PASSWORD, responseCommand, strlen(REQUIRED_PASSWORD)) == 0);
	int isLoggedIn = (strncmp(SUCCESS_LOGIN, responseCommand, strlen(SUCCESS_LOGIN)) == 0);

	if (!isAskingPassword && !isLoggedIn) {
		return logError("received invalid response from server, wrong username?...");
	}

	if (isAskingPassword) {

		// SEND "PASS" COMMAND
		if (!sendCommand(ftp->fdControl, passCommand, strlen(passCommand))) {
			return logError("sending PASS command to server failed!");
		}

		// CHECK IF COMMAND RETURN CODE IS VALID (AUTHENTICATION MODE)
		if (!anonymousMode && !receiveCommand(ftp->fdControl, SUCCESS_LOGIN)) {
			return logError("received invalid response from server, wrong password?");
		}

		// CHECK IF COMMAND RETURN CODE IS VALID (ANOYMOUS MODE)
		if (anonymousMode && !receiveCommand(ftp->fdControl, SUCCESS_LOGIN)) {
			return logError("received invalid response from server, no anonymous access?");
		}
	}

	return TRUE;
}*/

static void sendPASV(int fd) {

	char response[MESSAGE_SIZE + 1];

  if(sendCommand(fd, "PASV\r\n", response, 1) != 0){
    printf("Error entering passive mode. Exiting...\n");
    exit(1);
  }

  int values[6];
  char* data = strchr(response, '(');
  sscanf(data, "(%d, %d, %d, %d, %d, %d)", &values[0],&values[1],&values[2],&values[3],&values[4],&values[5]);
  sprintf(url->ip, "%d.%d.%d.%d", values[0],values[1],values[2],values[3]);
  url->port = values[4]*256+values[5];
}
	/*// SEND "PASV" COMMAND
	if (!sendCommand(fd, "PASV\r\n", strlen("PASV\r\n"))) {
		return logError("sending PASV command to server failed!");
	}

	char* pasvResponse = receiveCommand(fd, PASSIVE_MODE);
	int remoteIP[4];
	int remotePort[2];

	// CHECK IF COMMAND RETURN CODE IS VALID
	if (pasvResponse == NULL) {
		return logError("received invalid response from server, expected [221:PASSIVE_MODE]...");
	}

	if ((sscanf(pasvResponse, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &remoteIP[0], &remoteIP[1], &remoteIP[2], &remoteIP[3], &remotePort[0], &remotePort[1])) < 6) {
		return logError("attempt to parse remote address and port failed, invalid format?");
	}

	char* pasvHostname = (char*) malloc(strlen(pasvResponse) + 1);
	int pasvPort = remotePort[0] * 256 + remotePort[1];

	if ((sprintf(pasvHostname, "%d.%d.%d.%d", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3])) < 0) {
		return logError("attempt to generate remote IP address failed, invalid format?");
	}

	ftp->fdData = connectSocket(pasvHostname, pasvPort);

	if (ftp->fdData < 0) {
		return logError("connection to remote host refused!");
	}

	return TRUE;
}*/

int download(int fd) {
	FILE* outfile;
  if(!(outfile = fopen(url->filename, "w"))) {
		printf("ERROR: Cannot open file.\n");
		return 1;
	}

  char buf[1024]; //TODO -> usar file size da macro
  int bytes;
  while ((bytes = read(fd, buf, sizeof(buf)))) {
    if (bytes < 0) {
      printf("ERROR: Nothing was received from data socket fd.\n");
      return 1;
    }

    if ((bytes = fwrite(buf, bytes, 1, outfile)) < 0) {
      printf("ERROR: Cannot write data in file.\n");
      return 1;
		}
	}

	fclose(outfile);

  printf("Finished downloading file\n");

	return 0;
}

int quitConnection() {

	printf("Closing connection\n");
  if(sendCommand(ftp->fdControl, "QUIT\r\n", NULL, 0) != 0){
		printf("Error closing connection. Exiting anyway...\n");
		close(ftp->fdData);
		close(ftp->fdControl);
    exit(1);
  }

	close(ftp->fdData);
	close(ftp->fdControl);

  printf("Goodbye!\n");

	return 0;
}

	/*

	// SEND "QUIT" COMMAND
	if (!sendCommand(ftp->fdControl, "QUIT\r\n", strlen("QUIT\r\n"))) {
		return logError("sending QUIT command to server failed!");
	}

	if (ftp->fdData && close(ftp->fdData) < 0) {
		return logError("%s connection problem: attempt to disconnect failed.\n");
	}

	if (ftp->fdControl && close(ftp->fdControl) < 0) {
		return logError("%s connection problem: attempt to disconnect failed.\n");
	}

	destructURL(url);

	return TRUE;
}
*/

int startConnection(char* serverUrl) {

	ftp = (FTP *) malloc(sizeof(FTP));

	url = constructURL();

	setURLTestValues(url);

	fillIp(url);

  /*
	if (!parseURL(url, serverUrl)) {
		return FALSE;
	}
	*/

 	if((ftp->fdControl = connectSocket(url->ip, url->port)) == 0){
	 	printf("Error opening control connection\n");
	 	exit(1);
 	}

 	sendUSER(ftp->fdControl);
 	sendPASV(ftp->fdControl);

 	if((ftp->fdData = connectSocket(url->ip, url->port)) == 0){
	 	printf("Error opening data connection\n");
	 	exit(1);
 	}
 	retrieveFile(ftp->fdControl);
 	download(ftp->fdControl);
 	quitConnection();

	return 0;
}


	/*

	ftp->fdControl = connectSocket(url->ip, url->port);

	if (ftp->fdControl < 0) {
		perror("socket()");
		return FALSE;
	}

	if (!receiveCommand(ftp->fdControl, SERVER_READY)) {
		return logError("received invalid response from server, expected [220:SERVER_READY]");
	}

	if (!sendUSER(ftp->fdControl)) {
		return FALSE;
	}

	printf("2\n");


	if (!sendPASV(ftp->fdControl)) {
		return FALSE;
	}

	printf("3\n");

	if(!retrieveFile()) {
		return FALSE;
	}

	printf("4\n");

	return quitConnection();
}
*/
