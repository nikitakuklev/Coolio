#include "logic.h"
#include "errors.h"
#include "lcd.h"
#include "led.h"
#include "fanctrl.h"


// Contains all the business logic

void setMode(uint8_t mode) {
  if(GUImode == 0) {
    setNormalMode();    
  } else if (GUImode == 1) {
    setManualMode();
  } else if (GUImode == 2) {
    setDumbMode();
  } else {
    panic(100);
  }
}

void GUIloop() {  
  #if (DEBUG>1)
    Serial.println(F(" Entering GUI loop"));
  #endif

  // Mode update
  if (GUImode_changed) {
    uint32_t GUItime = micros();
    setMode(GUImode);
    // Have to update time here to avoid passing things to funcs
    GUItime_old = GUItime;
    GUImode_changed = false;
  }

  #if HASLCD  
    #if (DEBUG>1)
      Serial.println(F(" Updating LCD"));
    #endif
    updateLCD();
  #endif
}

void setNormalMode() {
  #if (DEBUG>1)
    Serial.println(F(" Setting normal mode"));
  #endif
  setLED_normal();
  #if HASLCD
    GUIrotation = true;
    GUItimeout = true;
    GUIstate_toplim = 5;
    GUIstate_botlim = 0;
    GUIstate = 0;
    GUI_update_required = true;
    LCD_update_period = LOOP_GUI_UPDT;
  #endif
}

void setManualMode() {
  #if (DEBUG>1)
    Serial.println(F(" Setting manual mode"));
  #endif
  setLED_manualmode();
  #if HASLCD
    GUIrotation = true;
    GUItimeout = true;
    GUIstate_toplim = 9;
    GUIstate_botlim = 6;
    GUIstate = 6;
    GUI_update_required = true;
    LCD_update_period = LOOP_GUI_UPDT;
  #endif
}

void setDumbMode() {
  #if (DEBUG>1)
    Serial.println(F(" Setting dumb mode"));
  #endif
  setLED_dumbmode();
  #if HASLCD
    GUIrotation = true;
    GUItimeout = false;
    GUIstate_toplim = 11;
    GUIstate_botlim = 11;
    GUIstate = 11;
    GUI_update_required = true;
    LCD_update_period = LOOP_GUI_UPDT_FAST;
  #endif
}

// Encoder
void buttonDown() {
  GUImode += 1;
  // 3 modes available
  if (GUImode > 2) {
    GUImode = 0;
  }
  GUImode_changed = true;
  fan_mode_changed = true;
  immediate_fan_updt_reqd = true;
}
void encoderIncrement() {
  #if (DEBUG>2)
    Serial.println("EINC");
  #endif
  if (GUImode == GUI_AUTO) {
    setNextGUIState();
    GUIrotation_skipnext = true;    
  } else if (GUImode == GUI_MANUAL) {
    manualFanCtrlIncrement();
  } else {
    manualFanCtrlIncrement();
  }
}
void encoderDecrement() {
  #if (DEBUG>2)
    Serial.println("EDEC");
  #endif
  if (GUImode == GUI_AUTO) {
    setPrevGUIState();
    GUIrotation_skipnext = true;    
  } else if (GUImode == GUI_MANUAL) {
    manualFanCtrlDecrement();
  } else {
    manualFanCtrlDecrement();
  }
}

// LED
void setLED_normal() {
  if (fan_state == 0) {
    setOKLed(LED_FLASH);
  } else {
    setOKLed(LED_FLASH);
  }
  if (lastErrorCode == 0 && systemCode == 0) {
    setErrLed(LED_OFF);
  } else {
    setErrLed(LED_ON);
  }
}

void setLED_nonfatalerr() {
  setErrLed(LED_ON);
}

void setLED_manualmode() {
  setOKLed(LED_ON);
  if (lastErrorCode == 0 && systemCode == 0) {
    setErrLed(LED_OFF);
  } else {
    setErrLed(LED_ON);
  }
}

void setLED_dumbmode() {
  setOKLed(LED_ON);
  if (lastErrorCode == 0 && systemCode == 0) {
    setErrLed(LED_OFF);
  } else {
    setErrLed(LED_ON);
  }
}

void setLED_panic() {
  setOKLed(LED_OFF);
  setErrLed(LED_BLINK_SLOW);
}
