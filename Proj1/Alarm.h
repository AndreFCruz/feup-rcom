#ifndef ALARM_H
#define ALARM_H

extern int alarmWentOff;

void setAlarmTimeout(uint secs);

void alarmHandler(int signal);

void setAlarm();

void stopAlarm();

#endif