#if HASLCD

static void setupLCD() {
  #if (DEBUG)
    Serial.println("Setting up LCD");
  #endif
  lcd.begin(16,2);
  lcd.setBacklight(HIGH);
  //lcd.print("Hi!");
  //lcd.blink();
  lcd.noCursor();
  lcd.noBlink();
  GUItime_old = micros();
  GUIstate_old = 255;
  LCDstate = 1;
}

static inline void setManualLCDMode() {
  #if (DEBUG>1)
    Serial.println(F("Setting manual LCD mode"));
  #endif
  setLED_manualmode();
  GUIrotation = true;
  GUItimeout = true;
  GUIstate_toplim = 9;
  GUIstate_botlim = 6;
  GUIstate = 6;
  GUItime_old = micros();
  GUI_update_required = true;
}

static inline void setNormalLCDMode() {
  #if (DEBUG>1)
    Serial.println(F("Setting normal LCD mode"));
  #endif
  // Normal mode
  setLED_normal();
  GUIrotation = true;
  GUItimeout = true;
  GUIstate_toplim = 5;
  GUIstate_botlim = 0;
  GUIstate = 0;
  GUItime_old = micros();
  GUI_update_required = true;
}

static inline void manualGUIreset() {
  if (GUImode) {
    // Back to start screen
    GUIstate = 6;
    GUItime_old = micros();
  }
}

static void GUIloop() {  
  #if (DEBUG>1)
    Serial.println(F("Entering GUI loop"));
  #endif
  if (GUImode_changed) {
    if (GUImode) {
      setManualLCDMode();
    } else {
      setNormalLCDMode();
    }
    GUImode_changed = false;
  }
  uint32_t GUItime = micros();
  // Do GUI loop 
  if (GUIrotation && LCDstate && (GUItime - GUItime_old > LCD_ROTTIME)) {
    GUItime_old = GUItime;    
    if (!GUIrotation_skipnext) {
      GUI_update_required = true; 
      setNextGUIState();    
      #if (DEBUG>2)
        Serial.print(F("Doing GUI rotation to ")); Serial.println(GUIstate); 
      #endif
    } else { 
      #if (DEBUG>2)
        Serial.println(F("Skipping 1 GUI rotation")); 
      #endif 
      GUIrotation_skipnext = false;
    }    
  }    
  // LCD timeout
  if (GUItimeout && (GUItime - lastButtonTime > LCD_TIMEOUT) && (GUItime - lastEncoderTime > LCD_TIMEOUT)) { 
    if (LCDstate) {
      lcd.off();
      LCDstate = 0; 
      #if (DEBUG>1)
        Serial.println(F("LCD OFF"));
      #endif
    }
  } else {
    if (!LCDstate) {
      lcd.on();
      LCDstate = 1;
      #if (DEBUG>1)
        Serial.println(F("LCD ON"));
      #endif
    }
  }  
  // GUI update
  if (GUI_update_required) { 
    #if (DEBUG>1)
      Serial.print(F("GUI UPDATE - "));                    Serial.print(GUItime);     Serial.print(" | ");
      Serial.print(encoder_down); Serial.print(" | ");  Serial.print(button_down); Serial.print(" | ");    
      Serial.println("");
    #endif
    #if (DEBUG>1)
    if (button_down) {   
      #if (DEBUG>1)   
        Serial.println(F("BUTTON DOWN UPDATE"));
      #endif
      //selectState = !selectState;
      //button_down = false;
    }
    if (encoder_down) {
      #if (DEBUG>1)
        Serial.println(F("ENCODER DOWN UPDATE"));
      #endif
      //selectState = !selectState;
      //encoder_down = false;
    }  
    #endif
//    if (GUIstate_old != GUIstate) {
//      GUIstate_old = GUIstate;
//      drawGUI();
//    }
    // Redraw every time - had errors otherwise (noise?)
    if (GUIstate_old != GUIstate) {
      GUIstate_old = GUIstate;
    }    
    #if (DEBUG>2)
      uint32_t t1 = micros();
    #endif
    drawGUI(); 
    #if (DEBUG>2)
      Serial.print("DRAW TIME - "); Serial.println(micros() - t1);
    #endif
    GUI_update_required = false;
    GUI_immediateupdreqd = false;
  } else {
    #if (DEBUG>1)
      Serial.println(F("No GUI updt reqd"));
    #endif
  }
}

static inline void setNextGUIState() {  
  // GUI state overflow 
  if (GUIstate >= GUIstate_toplim) {                
    GUIstate = GUIstate_botlim;
  } else {
    GUIstate++;
  }
}

static inline void setPrevGUIState() {
  if (GUIstate <= GUIstate_botlim) {                
    GUIstate = GUIstate_toplim;
  } else { 
   GUIstate--;
  }
}

static inline void drawFatalErrorScreen() {
  GUIrotation = GUItimeout = false;
  lcd.on(); lcd.setBacklight(HIGH);
  GUIstate = 10;
  drawGUI();
}

static void drawGUI() { 
  #if (DEBUG>2)
    Serial.print("DRAW GUI call: "); Serial.println(GUIstate);
  #endif 
  lcd.setCursor(0,0);
  lcd.print("                 "); 
  lcd.setCursor(0,1);
  lcd.print("                 ");
  lcd.setCursor(0,0);
  switch(GUIstate) {
    case 0:
      lcd.print(" 1:"); lcd.print(fanrpms[0]);
      lcd.setCursor(9,0);   lcd.print("2:"); lcd.print(fanrpms[1]);
      lcd.setCursor(3,1);   lcd.print(" IR:"); lcd.print(temp_IR);              
      break;
    case 1:
      lcd.print(" 3:");   lcd.print(fanrpms[2]);
      lcd.setCursor(9,0);   lcd.print("4:");    lcd.print(fanrpms[3]);    
      lcd.setCursor(3,1);   lcd.print("SNS:");  lcd.print(temp_sensor);
      break;
    case 2:
      lcd.print(" 5:");   lcd.print(fanrpms[4]);
      lcd.setCursor(9,0);   lcd.print("6:");    lcd.print(fanrpms[5]);      
      lcd.setCursor(2,1);   lcd.print("CHIP:"); lcd.print(temp_chip);    
      break;
    case 3:
      lcd.print(" 7:"); lcd.print(fanrpms[6]);
      lcd.setCursor(8,0);   lcd.print("V:"); lcd.print(fan_voltage);
      lcd.setCursor(0,1);   lcd.print("PWM:"); lcd.print(t1_pwm_a);      
      lcd.setCursor(8,1);   lcd.print("I:"); lcd.print(fan_current);
      break;
    case 4:
      lcd.print("ERR:"); lcd.print(systemCode,HEX);       
      lcd.setCursor(0,1);   lcd.print("STS:"); lcd.print(statusString);
      break;
    case 5:
      lcd.print("LE:"); lcd.print(lastErrorCode,HEX);
      lcd.setCursor(6,0);   lcd.print("N:"); lcd.print(errCnt);       
      lcd.setCursor(0,1);   lcd.print("LS:"); lcd.print(lastErrorString);
      break;
    case 6:
      lcd.setCursor(2,0);   lcd.print("MANUAL  MODE");       
      //lcd.setCursor(0,1);   lcd.print("PWM:"); lcd.print(BLANK_3CHAR); lcd.setCursor(4,1); lcd.print(t1_pwm_a);
      //lcd.setCursor(8,1);   lcd.print("RPM:"); lcd.print(BLANK_4CHAR); lcd.setCursor(12,1); lcd.print(fanrpmavg);
      lcd.setCursor(0,1);   lcd.print("PWM:"); lcd.print(t1_pwm_a);
      lcd.setCursor(8,1);   lcd.print("RPM:"); lcd.print(fanrpmavg);
      break;
    case 7:
      lcd.print("IR:"); lcd.print(temp_IR);
      lcd.setCursor(10,0);  lcd.print("V:"); lcd.print(fan_voltage);      
      lcd.setCursor(0,1);   lcd.print("SN:"); lcd.print(temp_sensor);
      lcd.setCursor(10,1);  lcd.print("I:"); lcd.print(fan_current);
      break;
    case 8:
      lcd.print("ERR:"); lcd.print(systemCode,HEX);       
      lcd.setCursor(0,1);   lcd.print("STS:"); lcd.print(statusString);
      break;
    case 9:
      lcd.print("LE:"); lcd.print(lastErrorCode,HEX);
      lcd.setCursor(6,0);   lcd.print("N:"); lcd.print(errCnt);       
      lcd.setCursor(0,1);   lcd.print("LS:"); lcd.print(lastErrorString);
      break;
    case 10:
      lcd.print(systemCode,HEX);lcd.print(exitCode,HEX);lcd.print("|"); lcd.print(statusString);    
      lcd.setCursor(0,1);lcd.print(fan_state);lcd.print("|");lcd.print(temp_IR);lcd.print(temp_sensor);
      break;
    default:
      lcd.setCursor(3,0);   lcd.print("__G U I__");       
      lcd.setCursor(3,1);   lcd.print("E R R O R");
      break;
  }
  //lcd.noCursor();
  //lcd.noBlink();
}
#endif
