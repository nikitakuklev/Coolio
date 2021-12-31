#include "globals.h"

extern void setupLCD();
void createBarChars();
extern void updateLCD();
extern void manualGUIreset();
extern void setNextGUIState();
extern void setPrevGUIState();
extern void setDefaultGUIState();
void drawFatalErrorScreen();
void drawBar(byte x, byte y, byte pixels);
void drawGUI();
