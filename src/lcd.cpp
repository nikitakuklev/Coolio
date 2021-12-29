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
  LCD_update_period = LOOP_GUI_UPDT;
  createBarChars();
}

static void createBarChars() {
  byte mask = 0x10;
  byte current=0;
  for (int cgchar=1; cgchar<=4; cgchar++) {
    current |= mask;
    mask=mask>>1;
    for (int i=0; i<8; i++){
      LCD_character_array[i]=current;
    }
    lcd.createChar(cgchar, LCD_character_array);
  }
  for (int i=0; i<8; i++) {
    LCD_character_array[i]=0x1f; // Create full block.
  }
  lcd.createChar(0, LCD_character_array);
}

static void updateLCD() {
  uint32_t GUItime = micros();

  // LCD timeout  
  if (GUItimeout && (GUItime - lastButtonTime > LCD_TIMEOUT) && (GUItime - lastEncoderTime > LCD_TIMEOUT)) { 
    if (LCDstate) {
      cli();
      lcd.off();
      sei();
      LCDstate = LCD_OFF; 
      #if (DEBUG>1)
        Serial.println(F(" LCD OFF"));
      #endif
    }
  } else {
    if (!LCDstate) {
      cli();
      lcd.on();
      sei();
      LCDstate = LCD_ON;
      #if (DEBUG>1)
        Serial.println(F(" LCD ON"));
      #endif
    }
  }
    
  // Rotate screen
  if (LCDstate == LCD_ON) {
    if (GUIrotation && (GUItime - GUItime_old > LCD_ROTTIME)) {
      GUItime_old = GUItime;    
      if (!GUIrotation_skipnext) {
        GUI_update_required = true; 
        if (GUImode == GUI_DUMB) {
          setDefaultGUIState();
          #if (DEBUG>2)
            Serial.print(F(" Defaulting GUI to menu ")); Serial.println(GUIstate); 
          #endif
        } else {
          setNextGUIState();
          #if (DEBUG>2)
            Serial.print(F(" Rotating GUI to menu ")); Serial.println(GUIstate); 
          #endif
        }
            

      } else { 
        #if (DEBUG>2)
          Serial.println(F(" Skipping 1 GUI rotation")); 
        #endif 
        GUIrotation_skipnext = false;
      } 
    }   
  }    
  
  // GUI update
  if (GUI_update_required) { 
    if (LCDstate != LCD_ON) {
      panic(100);
    }
    #if (DEBUG>1)
      Serial.print(F(" GUI upd reqd - "));                    Serial.print(GUItime);     Serial.print(" | ");
      Serial.print(encoder_down); Serial.print(" | ");  Serial.print(button_down); Serial.print(" | ");    
      Serial.println("");
    #endif
    #if (DEBUG>1)
      if (button_down) {   
        #if (DEBUG>1)   
          Serial.println(F(" Btn down upd"));
        #endif
        //selectState = !selectState;
        //button_down = false;
      }
      if (encoder_down) {
        #if (DEBUG>1)
          Serial.println(F(" Enc down update"));
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
      Serial.print(" Draw time - "); Serial.println(micros() - t1);
    #endif
    GUI_update_required = false;
    GUI_immediate_update = false;
  } else {
    #if (DEBUG>1)
      Serial.println(F(" GUI loop end - no updt reqd"));
    #endif
  }
}

static inline void manualGUIreset() {
  if (GUImode == GUI_MANUAL) {
    // Back to start screen
    GUIstate = 6;
    GUItime_old = micros();
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

static inline void setDefaultGUIState() {
  GUIstate = GUIstate_botlim;
}

static inline void drawFatalErrorScreen() {
  GUIrotation = GUItimeout = false;
  lcd.on();
  lcd.setBacklight(HIGH);
  GUIstate = 10;
  drawGUI();
}

// From https://www.best-microcontroller-projects.com/hitachi-hd44780.html
static inline void drawBar(byte x, byte y, byte pixels) {
  int i;
  byte blocks = pixels / 5; // 5 pixels wide per character.
  byte rem    = pixels % 5;
  for (i=0;i<blocks;i++) {
     lcd.setCursor(x+i,y);
     lcd.write(byte(0));
  }
  lcd.setCursor(x+i,y);
  if (rem!=0) {
      lcd.write(rem);
  }
}

static inline void drawBarPercent(uint8_t x, uint8_t y, uint8_t width, uint8_t percent) {
  uint8_t pixels = width * 5 * percent;
  drawBar(x, y, pixels);
}

static void drawGUI() { 
  #if (DEBUG>2)
    Serial.print(" Draw GUI state: "); Serial.println(GUIstate);
  #endif 
//  lcd.setCursor(0,0);
//  lcd.print("                 "); 
//  lcd.setCursor(0,1);
//  lcd.print("                 ");
//  lcd.setCursor(0,0);
  lcd.clear();
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
      lcd.setCursor(2,0);   lcd.print("MANUAL "); lcd.print(" MODE");       
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
      lcd.setCursor(0,1);lcd.print(fan_state);lcd.print("|");lcd.print(temp_IR);lcd.print("|");lcd.print(temp_sensor);
      break;
    case 11:
      lcd.print("D T:"); lcd.print(temp_IR); lcd.print(" P:"); lcd.print(t1_pwm_a);
      uint8_t temp_percent;
      if (temp_IR > FANCTRL_TEMP_TOP) {
        temp_percent = 80;
      } else if (temp_IR < 0) {
        temp_percent = 0;
      } else {
        temp_percent = (uint8_t) 5 * 16 * temp_IR / FANCTRL_TEMP_TOP;
      }    
      #if (DEBUG>2)
        Serial.print(" Dumb temp pix of 80: "); Serial.println(temp_percent);
      #endif 
      drawBar(0,1,temp_percent);
      break;
    case 12:
      lcd.print("DUMB PWM:"); lcd.print(t1_pwm_a);
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
