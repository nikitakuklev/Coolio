#include "globals.h"

extern void declareError(uint8_t code);
extern bool isFatal(uint8_t code);
extern void clearError();
extern void updateStatusString();
extern void panic(uint8_t err);