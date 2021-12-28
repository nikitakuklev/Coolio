#define GAP     (FANCTRL_TEMP_TOP - FANCTRL_TEMP_BOT)
#define DIV     10
#define FIMAX   11
#define I1(VAL) FANCTRL_MINCYCLE + ((FANCTRL_MAXCYCLE-FANCTRL_MINCYCLE)*VAL)/FIMAX
#define T1(VAL) FANCTRL_TEMP_BOT + ((FANCTRL_TEMP_TOP-FANCTRL_TEMP_BOT)*VAL)/(FIMAX-1)

static const uint8_t pwm_vals[FIMAX+2] = {0,FANCTRL_MINCYCLE,I1(1),I1(2),I1(3),I1(4),I1(5),I1(6),I1(7),I1(8),I1(9),I1(10),FANCTRL_MAXCYCLE};
static const uint8_t temp_vals[FIMAX+1] = {FANCTRL_TEMP_OFF,FANCTRL_TEMP_BOT,T1(1),T1(2),T1(3),T1(4),T1(5),T1(6),T1(7),T1(8),T1(9),FANCTRL_TEMP_TOP};

static void doFanCtrlUpdate() {
  #if (DEBUG>2)
    Serial.println(F(" Auto fan update"));
  #endif
  updatePWMvalues();
  if (fan_state != new_state) {   
    #if (DEBUG>1)
      Serial.print(F(" Fan state change to ")); Serial.print(new_state);
      Serial.print(F(" from ")); Serial.println(fan_state);
    #endif  
    if (new_state != 0 && fan_state == 0) {
      #if (DEBUG>1)
        Serial.println(F(" New state >0 - spinup/power on"));
      #endif
      setFanPower(true);
      spinup_required = true;
    } else if (new_state == 0 && fan_state != 0) { 
      #if (DEBUG>1)
        Serial.println(F(" New state is 0 - depowering"));
      #endif     
      setFanPower(false);
    }
    fan_state = new_state;
    setFansToPWM();   
  }
}

static inline void manualFanCtrlUpdate() {
  #if (DEBUG>2)
    Serial.print(F(" Manual fan update to ")); Serial.println(man_state);
  #endif
  new_state = man_state;
  t1_pwm_a = t1_pwm_b = pwm_vals[new_state];
  if (fan_state != new_state) {  
    #if (DEBUG>1)
      Serial.print(F(" Man fan state change to ")); Serial.print(new_state);
      Serial.print(F(" from ")); Serial.println(fan_state);
    #endif  
    if (new_state != 0 && fan_state == 0) {
      #if (DEBUG>1)
        Serial.println(F(" New state >0 - spinup/power on"));
      #endif
      setFanPower(true);
      spinup_required = true;
    } else if (new_state == 0 && fan_state != 0) {    
      #if (DEBUG>1)
        Serial.println(F(" New state is 0 - depowering"));
      #endif   
      setFanPower(false);
    }    
    fan_state = new_state;
    setFansToPWM();
  }
}

static inline void manualFanCtrlIncrement() {
  if (fan_state < FIMAX+1) {
    man_state = fan_state + 1;
  }
}

static inline void manualFanCtrlDecrement() {
  if (fan_state > 0) {
    man_state = fan_state - 1;
  }
}

static void updatePWMvalues() {
  #if (DEBUG>2)
    Serial.println(F(" Calculating new PWM state"));
  #endif
  int16_t temp = (int16_t) temp_IR;
//  int8_t new_state = 0;
  uint8_t nohyst_state = 0;
  uint8_t cycle = 0;
  // It would be nice to use branch hints but avr-gcc ignores them it seems
  // First determine which bin the temperature falls in
  if (temp <= FANCTRL_TEMP_OFF) {
    nohyst_state = 0;
  } else if (temp < FANCTRL_TEMP_BOT) {
    nohyst_state = 1;  
  } else if (temp >= FANCTRL_TEMP_TOP) {
    nohyst_state = FIMAX+1;
  } else {
    nohyst_state = (((temp - FANCTRL_TEMP_BOT)*DIV)/GAP)+2;    
  }
  
  // Next determine if should remain in old state if still within hyst range
  if (fan_state == nohyst_state) {                              // do nothing
    new_state = fan_state;                                    
  } else if (fan_state-nohyst_state == 1) {                     // check for hysteresis (downward)
    if ((temp_vals[nohyst_state]-FANCTRL_TEMP_HYST)>temp) {
      new_state = nohyst_state;       
    } else {
      #if (DEBUG>2)
        Serial.print(F(" HYST DN TRIG, KEEPING STATE ")); Serial.println(fan_state);
      #endif
      new_state = fan_state;
    }    
  } else if (nohyst_state-fan_state == 1) {                     // check for hysteresis (upward)
    if ((temp_vals[nohyst_state-1]+FANCTRL_TEMP_HYST)<temp) {
      new_state = nohyst_state;       
    } else {
      #if (DEBUG>2)
        Serial.print(F(" HYST UP TRIG, KEEPING STATE ")); Serial.println(fan_state);
      #endif
      new_state = fan_state;
    }  
  } else {
    new_state = nohyst_state;                                   // too far, no need to check hyst
  }
  #if (DEBUG>1)
    Serial.print(" NOHYST state: "); Serial.print(nohyst_state);
    Serial.print(" | NEW state: "); Serial.print(new_state);
    Serial.print(" | FAN state: "); Serial.println(fan_state);
  #endif
  // Finally, set appropriate PWM
//  if(new_state == 0) {
//    cycle = 0;
//  } else if(new_state == 1) {
//    cycle = FANCTRL_MINCYCLE;
//  } else if (new_state <= (FIMAX-1)) {
//    cycle = pwm_vals[new_state];
//  } else {
//    cycle = FANCTRL_MAXCYCLE;
//  }  
  cycle = pwm_vals[new_state];
  #if (DEBUG>1)
    Serial.print(F(" PWM value for temp ")); Serial.print(temp);
    Serial.print(F(" calculated as ")); Serial.println(cycle);
  #endif

  t1_pwm_a = cycle;
  t1_pwm_b = cycle;
}

static void fanCtrlAlarmCheck() {
  #if (DEBUG>1)
    Serial.print(F(" Checking fan alarms (active): ")); printBits(FANCTRL_ACTFANS); Serial.println("");
  #endif
  for (uint8_t i=0; i<FANARR_SIZE; ++i) {
    if (fanrpms[i]<FANCTRL_ALARMRPM && fan_state != 0 && (FANCTRL_ACTFANS & (1<<i))) {
      ++fanFailScores[i];
      #if (DEBUG>1)
        Serial.print(F("  Check FAIL (fan|rpm|cnt): ")); Serial.print(i); vline();
        Serial.print(fanrpms[i]); vline(); Serial.print(fanFailScores[i]); Serial.println("");
      #endif
      if (fanFailScores[i] > FANCTRL_FAILTHR) {
        declareError(FCTRL_RPMTOOLOW);
        fanFailScores[i] = FANCTRL_FAILTHR;
      }
    } else {
      fanFailScores[i] = 0;
    }
  }
}

static void setFansToMax() {
  #if (DEBUG>1)
    Serial.println(F(" Setting timer1 PWMs to MAX"));
  #endif
  analogWriteT1Raw(TIMER1_A_PIN,t1_topcnt);
  analogWriteT1Raw(TIMER1_B_PIN,t1_topcnt);
}

static void setFansToPWM() {
  #if (DEBUG>1)
    Serial.println(F(" Updating timer1 PWMs"));
  #endif
  analogWriteT1(TIMER1_A_PIN, t1_pwm_a);
  analogWriteT1(TIMER1_B_PIN, t1_pwm_b);
}

static void setupFanController() {
  #if (DEBUG)
    Serial.println(F("Setting up fan controller"));
  #endif
  #if (DEBUG>2)
    Serial.print(F("PWM values:  "));
    for(uint8_t i=0; i<=FIMAX+1; i++) {
      Serial.print(pwm_vals[i]); Serial.print("|");
    }
    Serial.println("");
    
    Serial.print(F("Temp values: "));
    for(uint8_t i=0; i<=FIMAX; i++) {
      Serial.print(temp_vals[i]); Serial.print("|");
    }
    Serial.println("");    
    Serial.print("STEP: "); Serial.println(GAP/DIV);
  #endif
  noInterrupts();
  // Set MOSFET pin low output
  FANCTRL_SW_PORT &= ~FANCTRL_SW_PIN;
  FANCTRL_SW_DDR  |= FANCTRL_SW_PIN;
  // Not needed but just in case...
  if (GUImode && man_state > 0) {
    spinup_required = true;
    enableFanPower();
  }
  interrupts();
}

static void setFanPower(bool state) {
  if (state) {
    enableFanPower();
  } else {
    disableFanPower();
  }
}

static void disableFanPower() {
  #if (DEBUG>1)
    Serial.println(" Fan power OFF");
  #endif
  FANCTRL_SW_PORT &= ~FANCTRL_SW_PIN;
}

static void enableFanPower() {
  #if (DEBUG>1)
    Serial.println(" Fan power ON");
  #endif
  FANCTRL_SW_PORT |= FANCTRL_SW_PIN;
}
