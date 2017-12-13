#include "clientFTP.h"
#include "Url.h"
#include "utils.h"

FTP * ftp;
URL * url;

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

static int retrieveFile(void) {

	char userCommand[MESSAGE_SIZE + 1];

	// SWITCH TO CURRENT WORKING DIRECTORY
	if (url->path != NULL && !sendCWD()) {
		return FALSE;
	}

	// FORMAT "RETR" COMMAND ARGUMENTS
	sprintf(userCommand, "RETR %s\r\n", url->filename);

	// SEND "RETR" COMMAND
	if (!sendCommand(ftp->fdControl, userCommand, strlen(userCommand))) {
		ERROR("sending RETR command to server failed!");
	}

	// CHECK IF COMMAND RETURN CODE IS VALID
	char* responseCommand = receiveCommand(ftp->fdControl, STATUS_OK);
	if (!responseCommand) {
		ERROR("received invalid response from server, file not found?");
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

	puts("[INFORMATION] starting file transfer...");

	int fd = open(url->filename, O_WRONLY | O_CREAT, 0600);

	if (fd < 0) {
		perror(url->filename);
		return FALSE;
	}

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
			ERROR("could not write data to output file!");
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

	if (close(fd) < 0) {
		perror(url->filename);
		return FALSE;
	}

	// CHECK IF COMMAND RETURN CODE IS VALID
	if (!receiveCommand(ftp->fdControl, "226")) {
		ERROR("received invalid response from server, connection interrupted?");
	}

	// CHECK EXPECTED FILE SIZE
	if (!unknownSize) {

		if (fileSize != bytesRead) {
			ERROR("expected and received file sizes don't match!");
		}

		transferSpeed = bytesSinceUpdate / (double) (currentUpdate - lastUpdate);
		logProgress(bytesRead, fileSize, transferSpeed);
	}

	puts("[INFORMATION] file transfer completed successfully!");
	printf("[INFORMATION] TOTAL BYTES RECEIVED: %d bytes\n", bytesRead);
	printf("[INFORMATION] AVERAGE TRANSFER SPEED: %.2f kBytes/sec\n", (double) bytesRead / totalTime);

	return TRUE;
}

static int sendCWD(void) {

	char userCommand[MESSAGE_SIZE + 1];

	// FORMAT "CWD" COMMAND ARGUMENTS
	sprintf(userCommand, "CWD %s\r\n", url->path);

	// SEND "CWD" (change working directory) COMMAND
	if (!sendCommand(ftp->fdControl, userCommand, strlen(userCommand))) {
		ERROR("sending CWD command to server failed!");
	}

	// CHECK IF COMMAND RETURN CODE IS VALID
	if (!receiveCommand(ftp->fdControl, DIRECTORY_OK)) {
		ERROR("received invalid response from server, target directory not found?");
	}

	printf("[INFORMATION] entering directory %s...\n", url->path);

	return TRUE;
}

static int sendUSER(int fd) {

	char userCommand[MESSAGE_SIZE + 1];
	char passCommand[MESSAGE_SIZE + 1];
	int anonymousMode = FALSE;

	// FORMAT "USER" COMMAND ARGUMENTS
	if (ftp->userName == NULL || strcmp("anonymous", ftp->userName) == 0) {
		puts("[INFORMATION] entering anonymous mode...");
		sprintf(userCommand, "USER %s\r\n", "anonymous");
		anonymousMode = TRUE;
	}
	else {
		puts("[INFORMATION] entering authentication mode...");
		sprintf(userCommand, "USER %s\r\n", ftp->userName);
	}

	if (ftp->userPassword == NULL && !anonymousMode) {
		ERROR("user must enter a password in authentication mode!");
	}

	// FORMAT "PASS" COMMAND ARGUMENTS
	sprintf(passCommand, "PASS %s\r\n", ftp->userPassword);

	// SEND "USER" COMMAND
	if (!sendCommand(ftp->fdControl, userCommand, strlen(userCommand))) {
		ERROR("sending USER command to server failed!");
	}

	// CHECK IF COMMAND RETURN CODE IS VALID
	char* responseCommand = receiveCommand(ftp->fdControl, NULL);
	int isAskingPassword = (strncmp(USER_OK, responseCommand, strlen(USER_OK)) == 0);
	int isLoggedIn = (strncmp(PASS_OK, responseCommand, strlen(PASS_OK)) == 0);

	if (!isAskingPassword && !isLoggedIn) {
		ERROR("received invalid response from server, wrong username?...");
	}

	if (isAskingPassword) {

		// SEND "PASS" COMMAND
		if (!sendCommand(ftp->fdControl, passCommand, strlen(passCommand))) {
			ERROR("sending PASS command to server failed!");
		}

		// CHECK IF COMMAND RETURN CODE IS VALID (AUTHENTICATION MODE)
		if (!anonymousMode && !receiveCommand(ftp->fdControl, PASS_OK)) {
			ERROR("received invalid response from server, wrong password?");
		}

		// CHECK IF COMMAND RETURN CODE IS VALID (ANOYMOUS MODE)
		if (anonymousMode && !receiveCommand(ftp->fdControl, PASS_OK)) {
			ERROR("received invalid response from server, no anonymous access?");
		}
	}

	return TRUE;
}

static int sendPASV(int fd) {

	// SEND "PASV" COMMAND
	if (!sendCommand(fd, "PASV\r\n", strlen("PASV\r\n"))) {
		ERROR("sending PASV command to server failed!");
	}

	char* pasvResponse = receiveCommand(fd, PASV_READY);
	int remoteIP[4];
	int remotePort[2];

	// CHECK IF COMMAND RETURN CODE IS VALID
	if (pasvResponse == NULL) {
		ERROR("received invalid response from server, expected [221:PASV_READY]...");
	}

	if ((sscanf(pasvResponse, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &remoteIP[0], &remoteIP[1], &remoteIP[2], &remoteIP[3], &remotePort[0], &remotePort[1])) < 6) {
		ERROR("attempt to parse remote address and port failed, invalid format?");
	}

	char* pasvHostname = (char*) malloc(strlen(pasvResponse) + 1);
	int pasvPort = remotePort[0] * 256 + remotePort[1];

	if ((sprintf(pasvHostname, "%d.%d.%d.%d", remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3])) < 0) {
		ERROR("attempt to generate remote IP address failed, invalid format?");
	}

	ftp->fdData = connectSocket(pasvHostname, pasvPort);

	if (ftp->fdData < 0) {
		ERROR("connection to remote host refused!");
	}

	return TRUE;
}

int quitConnection() {

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

int startConnection(char* serverUrl) {

	if (!parseURL(serverUrl)) {
		return FALSE;
	}

	ftp->fdControl = connectSocket(url->ip, url->port);

	if (ftp->fdControl < 0) {
		perror("socket()");
		return FALSE;
	}

	if (!receiveCommand(ftp->fdControl, SERVICE_READY)) {
		ERROR("received invalid response from server, expected [220:SERVICE_READY]");
	}

	if (!sendUSER(ftp->fdControl)) {
		return FALSE;
	}

	if (!sendPASV(ftp->fdControl)) {
		return FALSE;
	}

	retrieveFile();

	return quitConnection();
}
