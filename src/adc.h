#include "globals.h"

extern void setupADC();
void updatePowerData();
extern void enableADC();
extern void disableADC();
double getChipTemperature();
uint16_t getADCValue();
void adcAlarmCheck();
void setADCPin(uint8_t pin);
