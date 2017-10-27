#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "utils.h"
#include "Alarm.h"

#define DEFAULT_TIMEOUT		3


int alarmWentOff = FALSE;

static int alarmTimeout = DEFAULT_TIMEOUT;

void setAlarmTimeout(uint secs) {
	alarmTimeout = secs;
}

void alarmHandler(int signal) {
	if (signal != SIGALRM)
		return;

	alarmWentOff = TRUE;
	printf("\tConnection timed out!\n");

	alarm(alarmTimeout);
}

void setAlarm() {
	struct sigaction action;
	action.sa_handler = alarmHandler;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	sigaction(SIGALRM, &action, NULL);

	alarmWentOff = FALSE;

	alarm(alarmTimeout);
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
