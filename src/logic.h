#include "globals.h"

void setMode(uint8_t mode);
extern void GUIloop();
void setNormalMode();
void setManualMode();
void setDumbMode();
extern void buttonDown();
extern void encoderIncrement();
extern void encoderDecrement();
void setLED_normal();
void setLED_nonfatalerr();
void setLED_manualmode();
void setLED_dumbmode();
void setLED_panic();