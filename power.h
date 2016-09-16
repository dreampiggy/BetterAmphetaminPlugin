#include <psp2/power.h>

#include <stdio.h> 
#include <string.h>
#include <stdlib.h>	//atoi

/** Battery **/
int scePowerIsBatteryExist();
int scePowerGetBatteryTemp();
int scePowerGetBatteryVolt();
int scePowerGetBatterySOH();

/** Processor Clock **/
int scePowerGetGpuXbarClockFrequency();
void scePowerSetGpuXbarClockFrequency(int freq);

int getClockFrequency();
void setClockFrequency();

int getBatteryRemCapacity();
int getBatteryCapacity();
int getBatteryLifeTime();
char* getBatteryPercentage();
char* getBatteryVoltage();
char* getBatteryTempInCelsius();
char* getBatteryTempInFahrenheit();
const int getBatteryStatus();