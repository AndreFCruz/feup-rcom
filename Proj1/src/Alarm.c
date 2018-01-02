#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "utils.h"
#include "Alarm.h"


int alarmWentOff = FALSE;

void alarmHandler(int signal) {
	if (signal != SIGALRM)
		return;

	alarmWentOff = TRUE;
	printf("\tConnection timed out!\n");
}

void setAlarm() {
	struct sigaction action;
	action.sa_handler = alarmHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGALRM, &action, NULL);

	alarmWentOff = FALSE;
}

void stopAlarm() {
	struct sigaction action;
	action.sa_handler = NULL;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	alarmWentOff = FALSE;

	sigaction(SIGALRM, &action, NULL);

	alarm(0);
}
