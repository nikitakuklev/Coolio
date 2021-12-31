#include "globals.h"

extern void doFanCtrlUpdate();
extern void manualFanCtrlUpdate();
extern void manualFanCtrlIncrement();
extern void manualFanCtrlDecrement();
extern void updatePWMvalues();
extern void fanCtrlAlarmCheck();
extern void setFansToMax();
extern void setFansToPWM();
extern void setupFanController();
extern void setFanPower(bool state);
extern void disableFanPower();
extern void enableFanPower();