#include "globals.h"
#include "adc.h"
#include "eeprom.h"
#include "errors.h"
#include "extras.h"
#include "encoder.h"
#include "fanctrl.h"
#include "ir.h"
#include "lcd.h"
#include "led.h"
#include "logic.h"
#include "timer.h"
#include "tach.h"

#include "SoftWire.h"  
SoftWire Wire = SoftWire();

void setup() {
  #ifdef DEBUG  
    Serial.begin(115200); 
    #if (DEBUG > 1)
      Serial.print(F("Mr. Coolio starting on platform "));
      Serial.print(boot_signature_byte_get(0x00),HEX); Serial.print(" ");
      Serial.print(boot_signature_byte_get(0x02),HEX); Serial.print(" ");
      Serial.println(boot_signature_byte_get(0x04),HEX);
      Serial.print(F("Freq: ")); Serial.println(F_CPU);
      Serial.println("");
    #endif
    delay(500);
  #endif 

  //setup_test_IRsensor();
  //setup_test_IRandPWM();
  //setup_test_IR_PWM_TACH();
  //setup_test_IR_PWM_ADC();
  //setup_test_IR_PWM_TACH_LCD();
  //setup_test_IR_PWM_TACH_LCD_ENC();
  //setup_test_IR_PWM_TACH_LCD_ENC_LED();

  restoreEEPROMvals();
  setupLED();
  #if HASLCD
    setupLCD();
  #endif  
  setMode(GUImode);
  setupIRSensor();
  setupTimer1(TIMER1_DEF_FREQ);
  setupFanController();
  setupTach();  
  setupADC();
  updateStatusString();
  setupEncoderEXTINT0();  
  setupButtonEXTINT1();
  enableEncoderEXTINT0();
  enableButtonEXTINT1();
}

void loop() {
  //loop_test_IRsensor();
  //loop_test_IRandPWM();
  //loop_test_IR_PWM_TACH();
  //loop_test_IR_PWM_ADC();
  //loop_test_IR_PWM_TACH_LCD();
  //loop_test_IR_PWM_TACH_LCD_ENC();
  //loop_test_IR_PWM_TACH_LCD_ENC_LED();  
  uint32_t main_time = millis();
  uint32_t main_time_micros = micros();
  uint8_t cleanup = false;
  #if (DEBUG) 
    uint8_t activity = false;
  #endif

  // Temp update
  if (main_time > temp_update_time) {
    #if (DEBUG)      
      Serial.println(F("TEMP UPD - TICK"));
      activity = true;
    #endif
    updateTemps();   
    temp_update_time += LOOP_TEMP_UPDT;
    #if (DEBUG)
      printTemps();
    #endif
    tempAlarmCheck();
  }
  
  // Fan control update
  if (GUImode == GUI_AUTO) {
    // Automatic fan control
    if (main_time > fanctrl_update_time || immediate_fan_updt_reqd) {
      #if (DEBUG)
        Serial.println(F("AUTO FC UPD"));
        activity = true;
      #endif   
      doFanCtrlUpdate();  
      // If just changing into auto, need to reset last update time   
      if (fan_mode_changed) {
        #if (DEBUG)
          Serial.println(F(" Adjusting upd time to current"));
        #endif 
        fanctrl_update_time = main_time + LOOP_FANCTRL_UPDT;
        fan_mode_changed = false;
      } else {
        fanctrl_update_time += LOOP_FANCTRL_UPDT;
      }        
      immediate_fan_updt_reqd = false;
    } 
  } else if (GUImode == GUI_MANUAL) { 
    // If PWM target was modified by manual mode, need to adjust
    if (manualpwm_changed || immediate_fan_updt_reqd) {
      #if (DEBUG)
        Serial.println(F("MAN FC UPD"));
        activity = true;
      #endif      
      manualFanCtrlUpdate();
      manualGUIreset();
      manualpwm_changed = false;
      immediate_fan_updt_reqd = false;
      GUI_update_required = true;   
      GUI_immediate_update = true;
    }
  } else if (GUImode == GUI_DUMB) {
    if (manualpwm_changed || immediate_fan_updt_reqd) {
      #if (DEBUG)
        Serial.println(F("DUMB FC UPD"));
        activity = true;
      #endif      
      manualFanCtrlUpdate();
      manualGUIreset();
      manualpwm_changed = false;
      immediate_fan_updt_reqd = false;
      GUI_update_required = true;   
      GUI_immediate_update = true;
    } 
  }
    
  // Tachometer update
  if (main_time > tach_update_time) {  
    #if (DEBUG)      
      Serial.println(F("TACH UPD - TICK"));
      printFanDataMore();
      activity = true;
    #endif  
    doTachRun(100000);    
    tach_update_time += LOOP_TACH_UPDT;
    fanCtrlAlarmCheck();    
  }
  
  // Power consumption update
  if (main_time > adc_update_time) {  
    #if (DEBUG)
      Serial.println(F("ADC UPD - TICK"));
      activity = true;
    #endif    
    updatePowerData();
    adcAlarmCheck();
    adc_update_time += LOOP_ADC_UPDT;
  }  
  
  // EEPROM update
//  if ((main_time_micros - lastButtonTime > LCD_TIMEOUT) && (main_time_micros - lastEncoderTime > LCD_TIMEOUT)) {
//    // If no input for awhile, check if we should try to update again
//    if (main_time > lastEEPROMSaveTime + EEPROM_SVDELAY && (eeprom_guimode != GUImode || eeprom_manstate != man_state)) {
//      #if (DEBUG)
//        Serial.println(F("EEPROM UPD - TICK"));
//        activity = true;
//      #endif
//      //updateEEPROMvals();
//    }
//  }
    
  // GUI loop update  
  if (main_time > gui_update_time) { 
    #if (GUI_DEBUG)
      Serial.println(F("GUI UPD - TICK"));
      activity = true;
    #endif       
    GUIloop();    
    gui_update_time += LOOP_GUI_UPDT;
  } else if (GUI_immediate_update) {
    #if (GUI_DEBUG)
      Serial.println(F("GUI UPD - IMM"));
      activity = true;
    #endif
    GUIloop();
    gui_update_time = main_time + LOOP_GUI_UPDT;
    GUI_immediate_update = false;
  }

  // LED update
  if (main_time > led_update_time || led_immediate_update) {    
    #if (LED_DEBUG>2)
      Serial.println(F("LED UPD - TICK"));
      activity = true;
    #endif
    updateLED();
    if (led_immediate_update) {
      led_immediate_update = false;
      led_update_time = main_time + LOOP_LED_UPDT;
    } else {
      led_update_time += LOOP_LED_UPDT;
    }
  }

  if (spinup_required) {
    #if (DEBUG)
      Serial.println(F("SPINUP START"));
      activity = true;
    #endif
    setFansToMax();
    delay(1000);
    setFansToPWM();
    spinup_required = false;
    #if (DEBUG)
      Serial.println(F("SPINUP DONE"));
      activity = true;
    #endif 
  }  

  if (cleanup) {
    tachCleanup();
    cleanup = false;
  }
  #if (DEBUG)
    if (activity) {
      Serial.print("--------------------"); 
      Serial.print(millis()); Serial.print("|"); Serial.println(micros()-main_time_micros);
    }
  #endif


}



//void setup() {
//  pad[0];
//  // All ports start tristated (input, low)
//  //DIDR0 = _BV(ADC7D) | _BV(ADC6D); // Disable A6/A7 digital input
//  //DIDR0 = 0xFF; //Disable all digital inputs on analog bank (to reduce power)  
//  
//  #ifdef DEBUG  
//  Serial.begin(115200);                 // initialize serial communication
//  
//  #if (DEBUG > 1)
//  Serial.print(F("Mr. Coolio starting on platform "));
//  Serial.print(boot_signature_byte_get(0x00),HEX); Serial.print(" ");
//  Serial.print(boot_signature_byte_get(0x02),HEX); Serial.print(" ");
//  Serial.println(boot_signature_byte_get(0x04),HEX);
//  Serial.print(F("Freq: ")); Serial.println(F_CPU);
//  Serial.print(F("Temp: ")); Serial.println(getTemperature());
//  #endif
//  #endif
//
//  #if HASLCD  
//  lcd.begin(16,2); 
//  #endif
//
//  pinMode(LED_BUILTIN,OUTPUT);
//  pinMode(TIMER1_A_PIN, OUTPUT);
//  pinMode(TIMER1_B_PIN, OUTPUT);
//  digitalWrite(TIMER1_A_PIN, HIGH);
//  digitalWrite(TIMER1_B_PIN, HIGH);  
//  //pinMode(TACH_L_A_PIN, INPUT_PULLUP);
//  //pinMode(TACH_L_B_PIN, INPUT_PULLUP); 
//
//  noInterrupts();
//  setupTimer1(TIMER1_DEF_FREQ);
//  interrupts();
//}
//
////***************************************************************************************************************
//
//uint8_t pwmtestvals[] = {100,90,70,50,30,20,10,5};
//
//void loop() {   
//  printT1Registers();  
//  Serial.print("PWMFr:"); Serial.print(16000000/t1_topcnt/2);  
//  Serial.print(" | PWMVal:"); Serial.print(t1_pwm_a);  
//  Serial.print(" | OCR1A:"); Serial.print(OCR1A); 
//  Serial.print(" | OCR1B:"); Serial.print(OCR1B); 
//  Serial.print(" | TOPCNT:"); Serial.println(t1_topcnt);  
//  
//  uint8_t pwmindex = 0;
//  lcd.clear();
//  analogWriteT1(TIMER1_A_PIN,100);
//  delay(15000);
//  while (pwmindex <= 7) {
//    analogWriteT1(TIMER1_A_PIN,pwmtestvals[pwmindex]);
//    delay(15000);
//    
//    setupPCTachV2(200000);
//    delay(3000);
//    setupPCTachV2(100000);
//    delay(3000);
////    setupPCTachV2(200000);
////    delay(3000);
////    setupPCTachV2(100000);
////    delay(3000);    
////    setupPCTachV2(200000);
////    delay(3000);
//    for(uint8_t i=0; i<PORTD_SIZE; ++i) {
////      Serial.print(i); Serial.print(" | ");      
////      Serial.print(times[i]); Serial.print(" | ");      
////      if (numpts[i] > 0) {
////        Serial.print(toRPM(times[i],numpts[i]));
////      } else {
////        Serial.print(0);
////      }
////      Serial.print(" | ");
////      Serial.println(numpts[i]);  
//      if (numpts[i] > 0) {
//        Serial.print(i); Serial.print(" | ");      
//        Serial.print(times[i]); Serial.print(" | ");           
//        Serial.print(toRPM(times[i],numpts[i])); Serial.print(" | ");       
//        Serial.println(numpts[i]);  
//      }    
//    }
//    Serial.println("");
//      
//    #if HASLCD
////    lcd.clear();
////    for(uint8_t i=0; i<PORTD_SIZE; ++i) {
////      if (numpts[i] > 0) {
////        String st = "F";
////        lcd.print(st + i + ":" + pwmtestvals[pwmindex] + ":" + toRPM(times[i],numpts[i]));       
////      }
////    }
//      if (numpts[7] > 0) {
//        lcd.print(toRPM(times[7],numpts[7]));
//        if(toRPM(times[7],numpts[7]) < 1000) {
//          lcd.print(" ");
//        } 
//      } else {
//          lcd.print("0   ");
//      }
//    #endif
//    
//    // We need to clean up volatiles, but can't use memset because 
//    // "hurr durr C++ undefined behaviour casting away volatile modifier"
//    // so need to write our own loop and hope it gets optimized 
//    for(uint8_t i=0; i<PORTD_SIZE; ++i) {
//      times[i] = 0; numpts[i] = 0;      
//    }    
//    numISRcalls = 0; PCINT2_err = 0;
//    ISRtime = 0; ISRfirsttime = 0;    
//                      
//    pwmindex++;
//    if (pwmindex == 4) {
//      lcd.setCursor(0,1);
//    }
//  }
//  analogWriteT1(TIMER1_A_PIN,0);
//  delay(60000);
//}
