#ifndef ALARM_H
#define ALARM_H

extern int alarmWentOff;

void alarmHandler(int signal);

void setAlarm();

void stopAlarm();

#endif