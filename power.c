#include "power.h"

/** Processor Clock **/
int getClockFrequency(int no) {
	if (no == 0) return scePowerGetArmClockFrequency();
	else if (no == 1)	return scePowerGetBusClockFrequency();
	else if (no == 2)	return scePowerGetGpuClockFrequency();
	else if (no == 3)	return scePowerGetGpuXbarClockFrequency();
	else return 0;
}

void setClockFrequency(int no, int freq) {
	if (no == 0)	scePowerSetArmClockFrequency(freq);
	else if (no == 1)	scePowerSetBusClockFrequency(freq);
	else if (no == 2)	scePowerSetGpuClockFrequency(freq);
	else if (no == 3)	scePowerSetGpuXbarClockFrequency(freq);
}

/** Battery **/
const int getBatteryStatus() {
	return scePowerIsBatteryCharging();
}

int getBatteryRemCapacity() {
	char mAh[10];
	sprintf(mAh,"%i",scePowerGetBatteryRemainCapacity());
	int cap = atoi(mAh);	
	return cap;
}
int getBatteryCapacity() {
	char mAh[10];
	sprintf(mAh,"%i",scePowerGetBatteryFullCapacity());
	int cap = atoi(mAh);	
	return cap;
}

int getBatteryLifeTime() {
	char life_time[10];
	sprintf(life_time,"%i",scePowerGetBatteryLifeTime());
	int min = atoi(life_time);
	return min;
}

char* getBatteryPercentage() {
	static char percentage[5];
	sprintf(percentage, "%3d%%", scePowerGetBatteryLifePercent());
	return percentage;
}

char* getBatteryVoltage() {
	static char voltage[8];
	sprintf(voltage,"%0.2f",(float)scePowerGetBatteryVolt() / 1000.0);
	return voltage;
}

char* getBatteryTempInCelsius() {
	static char temp[8];
	sprintf(temp,"%0.2f",(float)scePowerGetBatteryTemp() / 100.0);
	return temp;
}
char* getBatteryTempInFahrenheit() {
	static char temp[8];
	sprintf(temp,"%0.2f",(1.8 * (float)scePowerGetBatteryTemp() / 100.0) + 32);
	return temp;
}