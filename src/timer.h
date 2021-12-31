#include "globals.h"

void analogWriteT1Raw(uint8_t pin, uint16_t value);
void analogWriteT1(uint8_t pin, uint32_t value);
extern void setupTimer1(uint16_t freq);
extern void setTimerPinsHigh();
extern void stopTimer1();
extern void resumeTimer1();