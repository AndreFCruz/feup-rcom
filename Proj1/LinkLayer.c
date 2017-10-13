#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// TODO filtrar estes includes

static struct linkLayer globalPtr;

int byteStuffing(char* buffer, int* length) {
	
}

int byteDestuffing(char* buffer, int* length) {

}


int llwrite(int fd, char * buffer, int length) {
	//adição de stuffing ao buffer
	//adição de header e trailer ao buffer.
	//escrever a nova mensagem
}

int llread(int fd, char * buffer) {
	// adição de header e trailer ao buffer.
	//remoção de stuffing ao buffer
	//escrever a nova mensagem
}
