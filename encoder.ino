ISR (INT0_vect) {
  static bool stage = true;  
  uint32_t timeNow = micros();
  //uint32_t deltat = timeNow - lastTrigTime;
  if (timeNow - lastEncoderTime > ENC_ENCDB) {  
    EICRA ^= _BV(ISC00);
    uint8_t datapinrg = ENC_DATAPINRG;
    #if (DEBUG>2)
      Serial.print(timeNow); Serial.print("E"); Serial.println(stage);
    #endif
    encoder_down = stage;
    //GUI_update_required = !stage;       

    lastEncoderTime = timeNow;
    // On falling clock edge, which indicates ending turn, measure data pin
    if (!stage) {
      GUI_update_required = true;   
      GUI_immediateupdreqd = true;
      //_delay_ms(1.0);
      #if (DEBUG>2)
        printBits(datapinrg); Serial.print("|");
        printBits(ENC_DATADDR);
      #endif
      if (datapinrg & ENC_DATAPIN) {
        encoderIncrement();
      } else {
        encoderDecrement();
      }
    }
    stage = !stage;
  }
}

static inline void encoderIncrement() {
  #if (DEBUG>2)
    Serial.println("EINC");
  #endif
  if (GUImode) {
    // Increment pwm
//    if (t1_pwm_a < FANCTRL_MAXCYCLE) {
//      t1_pwm_a += 1;
//      t1_pwm_b = t1_pwm_a;
//    }
    manualFanCtrlIncrement();
    manualpwm_changed = true;
  } else {
    setNextGUIState();
    GUIrotation_skipnext = true;
  }
}

static inline void encoderDecrement() {
  #if (DEBUG>2)
    Serial.println("EDEC");
  #endif
  if (GUImode) {
//    if (t1_pwm_a > FANCTRL_MINCYCLE) {
//      t1_pwm_a -= 1;
//      t1_pwm_b = t1_pwm_a;
//    }
    manualFanCtrlDecrement();
    manualpwm_changed = true;
  } else {
    setPrevGUIState();
    GUIrotation_skipnext = true;
  }
}

static void setupEncoderEXTINT0() {
  #if (DEBUG)
    Serial.println(F("Setting up encoder"));
  #endif
  ENC_CLKDDR &= ~ENC_CLKPIN;                    // Set encoder pins to input
  ENC_DATADDR &= ~ENC_DATAPIN;
  ENC_CLKPORT &= ~ENC_CLKPIN;
  ENC_DATAPORT &= ~ENC_DATAPIN;
  //PORTD |= B10000100;                         // Enable pullups
  EIMSK &= ~B00000001;                          // Disable INT0
  EIFR |= _BV(INTF0);                           // Clear external interrupt 0 flag
  EICRA |= _BV(ISC01); EICRA &= ~_BV(ISC00);    // ISC0 = 10 (falling on pin 0)
  //EICRA |= (_BV(ISC01) | _BV(ISC00));         // ISC0 = 11 (rising on pin 0)  

}
static inline void enableEncoderEXTINT0() {
  #if (DEBUG>2)
    Serial.println(F("Enabling encoder"));
  #endif
  EIMSK |= B00000001;                   // Enable INT0
  lastEncoderTime = micros();
}
static inline void disableEncoderEXTINT0() {
  EIMSK &= ~B00000001;                  // Disable INT0
}

//****************************************************

ISR (INT1_vect) {
  static bool stage = true;  
  uint32_t timeNow = micros();
  //uint32_t deltat = timeNow - lastTrigTime;
  if (timeNow - lastButtonTime > ENC_BTNDB) {
    EICRA ^= _BV(ISC10);            // toggle falling/rising  
    #if (DEBUG>2)
      Serial.print(timeNow); Serial.print("B"); Serial.println(stage);
    #endif
    if (stage) {  
      button_down = true;
      GUI_update_required = true;  
      GUI_immediateupdreqd = true; 
      buttonDown();
    } else {
      button_down = false;
    }
    stage = !stage;
    lastButtonTime = timeNow;          
  }
}

static inline void buttonDown() {
  GUImode ^= 0x01;
  GUImode_changed = true;
  immediate_fan_updt_reqd = true;
//  if(GUImode) {
//    // Manual mode
//    setManualLCDMode();
//  } else {
//    setNormalLCDMode();
//  }
}

static void setupButtonEXTINT1() { 
  #if (DEBUG)
    Serial.println(F("Setting up button"));
  #endif
  ENC_BTNDDR &= ~ENC_BTNPIN;                  // Set port D pin 3 to input
  ENC_BTNPORT &= ~ENC_BTNPIN;                  // Disable pullups 
  EIMSK &= ~B00000010;                        // Disable INT1
  EICRA |= _BV(ISC11); EICRA &= ~_BV(ISC10);  // ISC1 = 10 (falling on pin 1)
  EIFR |= _BV(INTF1);                         // Clear external interrupt flag 1  
}
static inline void enableButtonEXTINT1() {
  #if (DEBUG>2)
    Serial.println(F("Enabling button"));
  #endif
  EIMSK |= B00000010;                   // Enable INT1
  lastButtonTime = micros();
}
static inline void disableButtonEXTINT1() {
  EIMSK &= ~B00000010;                  // Disable INT1
}
//static inline void setButtonFallingEdge() {
//  EICRA |= _BV(ISC11); EICRA &= ~_BV(ISC10);
//}
//static inline void setButtonRisingEdge() {
//  EICRA |= _BV(ISC11) | _BV(ISC10);
//}
