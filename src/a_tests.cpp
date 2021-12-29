//inline void setup_test_IR_PWM_TACH_LCD_ENC_LED() {
//  restoreEEPROMvals();
//  setupLED();
//  setupIRSensor();
//  setupTimer1(TIMER1_DEF_FREQ);
//  setupFanController();
//
//  setupTach();  
//  #if HASLCD
//    setupLCD();
//    if(GUImode) {
//      setManualLCDMode();
//    } else {
//      setNormalLCDMode();
//    }
//  #endif  
//  setupADC();
//  updateStatusString();
//  setupEncoderEXTINT0();
//  enableEncoderEXTINT0();
//  setupButtonEXTINT1();
//  enableButtonEXTINT1();
//  Serial.println(F("**********************************"));
//}
//
//inline void loop_test_IR_PWM_TACH_LCD_ENC_LED() {
//  uint32_t main_time = millis();
//  uint32_t main_time_micros = micros();
//  uint32_t t1; uint32_t t2;
//  uint8_t cleanup = false;
//  if (main_time > temp_update_time) {
//    t1 = micros();
//    updateTemps();       
//    t2 = micros();
//    Serial.print(F("TEMP UPDATE IN ")); Serial.println(t2-t1);
//    temp_update_time += LOOP_TEMP_UPDT;
//    printTemps();
//    tempAlarmCheck();
//    Serial.println("---");
//  }
//  
//  // Fan control update
//  if (!GUImode) {
//    if (main_time > fanctrl_update_time || immediate_fan_updt_reqd) {
//      noInterrupts();
//      t1 = micros();      
//      doFanCtrlUpdate();
//      //setFansToPWM();
//      t2 = micros();      
//      fanctrl_update_time += LOOP_FANCTRL_UPDT;  
//      immediate_fan_updt_reqd = false;    
//      interrupts();
//      Serial.print(F("FANCTRL UPDATE IN ")); Serial.println(t2-t1);
//      Serial.println("---");
//    } 
//  } else { 
//    // If PWM target was modified by manual mode
//    if (manualpwm_changed || immediate_fan_updt_reqd) {
//      noInterrupts();
//      t1 = micros(); 
//      manualFanCtrlUpdate();
//      //setFansToPWM();
//      manualGUIreset();
//      manualpwm_changed = false;
//      immediate_fan_updt_reqd = false;
//      t2 = micros();
//      interrupts();
//      Serial.print(F("MAN FANCTRL UPDATE IN ")); Serial.println(t2-t1);
//      Serial.println("---");
//    }
//  }
//  if (spinup_required) {
//    setFansToMax();
//    delay(50);
//    setFansToPWM();
//    spinup_required = false;
//    Serial.println(F("SPINUP DONE"));
//    Serial.println("---");
//  }  
//  
//  // Tachometer update
//  if (main_time > tach_update_time) {
//    t1 = micros();      
//    doTachRun(100000);
//    t2 = micros();
//    Serial.print(F("TACH UPDATE IN ")); Serial.println(t2-t1);
//    tach_update_time += LOOP_TACH_UPDT;
//    printFanDataMore();
//    fanCtrlAlarmCheck();
//    Serial.println("---");
//  }
//  
//  // Power consumption update
//  if (main_time > adc_update_time) {
//    t1 = micros();      
//    updatePowerData();
//    t2 = micros();
//    Serial.print(F("ADC UPDATE IN ")); Serial.println(t2-t1);
//    adc_update_time += LOOP_ADC_UPDT;
//    Serial.println("---");
//  }  
//  
//  // EEPROM update
//  if ((main_time_micros - lastButtonTime > LCD_TIMEOUT) && (main_time_micros - lastEncoderTime > LCD_TIMEOUT)) {
//    // If no input for awhile, check if we should try to update again
//    if (main_time > lastEEPROMSaveTime + EEPROM_SVDELAY && (eeprom_guimode != GUImode || eeprom_manstate != man_state)) {
//      t1 = micros();
//      updateEEPROMvals();
//      t2 = micros();
//      Serial.print(F("EEPROM UPDATE IN ")); Serial.println(t2-t1);
//    } else {
//      Serial.println(F("NO EEPROM UPDATE"));
//    }
//    Serial.println("---");
//  }
//    
//  // GUI loop update
//  if (main_time > gui_update_time) {
//    t1 = micros();      
//    #if HASLCD
//      GUIloop(); 
//    #endif    
//    t2 = micros();
//    Serial.print(F("GUI UPDATE IN ")); Serial.println(t2-t1);
//    gui_update_time += LOOP_GUI_UPDT;
//  }  
//
//  if (main_time > led_update_time) {
//    t1 = micros(); 
//    updateLED();
//    t2 = micros();
//    Serial.print(F("LED UPDATE IN ")); Serial.println(t2-t1);
//    led_update_time += LOOP_LED_UPDT;
//  }
//
//  if (cleanup) {
//    tachCleanup();
//    cleanup = false;
//  }
//  Serial.print("--------------------"); Serial.print(millis()); Serial.print("|"); Serial.println(millis()-main_time);;
//  delay(1);
//}

//*****************************************************************

//inline void setup_test_IRsensor() {
//  setupIRSensor();  
//}
//
//inline void loop_test_IRsensor() {
//  uint32_t t1 = micros();
//  updateTemps();
//  uint32_t t2 = micros();
//  Serial.print("IR: "); Serial.print(temp_IR);
//  Serial.print(" | SENS: "); Serial.print(temp_sensor);
//  Serial.print(" | CHIP: "); Serial.print(temp_chip);
//  Serial.print(" | T: "); Serial.println(t2-t1);
//  Serial.println("");
//  delay(5000);
//}

//*****************************************************************

//inline void setup_test_IRandPWM() {
//  setupIRSensor();
//  setupTimer1(TIMER1_DEF_FREQ);
//  setupFanController();
//}
//
//inline void loop_test_IRandPWM() {
//  uint32_t t1 = micros();
//  updateTemps();  
//  doFanCtrlUpdate();  
//  uint32_t t2 = micros();
//  Serial.print("IR: "); Serial.print(temp_IR);
//  Serial.print(" | SENS: "); Serial.print(temp_sensor);
//  Serial.print(" | CHIP: "); Serial.print(temp_chip);
//  Serial.print(" | T: "); Serial.println(t2-t1);
//  Serial.println("");
//  delay(500);
//}

//*****************************************************************

//inline void setup_test_IR_PWM_TACH_LCD_ENC() {
//  setupIRSensor();
//  setupTimer1(TIMER1_DEF_FREQ);
//  setupFanController();
//  setupTach();  
//  #if HASLCD
//    setupLCD();
//  #endif
//  updateStatusString();
//  setupButtonEXTINT1();
//  enableButtonEXTINT1();
//  Serial.println("**********************************");
//}
//
//inline void loop_test_IR_PWM_TACH_LCD_ENC() {
//  uint32_t t1 = micros();
//  updateTemps();  
//  doFanCtrlUpdate();
//  uint32_t t2 = micros();
//  delay(500);
//  uint32_t t3 = micros();  
//  doTachRun(100000);
//  uint32_t t4 = micros();  
//  fanCtrlAlarmCheck();
//  Serial.print("IR: "); Serial.print(temp_IR);
//  Serial.print(" | SENS: "); Serial.print(temp_sensor);
//  Serial.print(" | CHIP: "); Serial.print(temp_chip);
//  Serial.print(" | T12: "); Serial.print(t2-t1);
//  Serial.print(" | T23: "); Serial.println(t4-t3);
//  //Serial.println("");
//  for(uint8_t i=0; i<FANARR_SIZE; ++i) {
////    Serial.print(i); Serial.print(" | ");      
////    Serial.print(times[i]); Serial.print(" | ");      
////    if (numpts[i] > 0) {
////      Serial.print(toRPM(times[i],numpts[i]));
////    } else {
////      Serial.print(0);
////    }
////    Serial.print(" | ");
////    Serial.println(numpts[i]);  
//    if (numpts[i] > 0) {
//      Serial.print(i); Serial.print(" | ");      
//      Serial.print(times[i]); Serial.print(" | ");           
//      Serial.print(fanrpms[i]); Serial.print(" | ");       
//      Serial.println(numpts[i]);  
//    }    
//  } 
//  #if HASLCD
//   GUIloop(); 
//  #endif
//  tachCleanup();  
//  Serial.println("---");
//  delay(5000);
//}

////*****************************************************************
//
//inline void setup_test_IR_PWM_TACH_LCD() {
//  setupIRSensor();
//  setupTimer1(TIMER1_DEF_FREQ);
//  setupFanController();
//  setupTach();  
//  #if HASLCD
//    setupLCD();
//  #endif
//  updateStatusString();
//  Serial.println("**********************************");
//}
//
//inline void loop_test_IR_PWM_TACH_LCD() {
//  uint32_t t1 = micros();
//  updateTemps();  
//  doFanCtrlUpdate();
//  uint32_t t2 = micros();
//  delay(500);
//  uint32_t t3 = micros();  
//  doTachRun(100000);
//  uint32_t t4 = micros();  
//  fanCtrlAlarmCheck();
//  Serial.print("IR: "); Serial.print(temp_IR);
//  Serial.print(" | SENS: "); Serial.print(temp_sensor);
//  Serial.print(" | CHIP: "); Serial.print(temp_chip);
//  Serial.print(" | T12: "); Serial.print(t2-t1);
//  Serial.print(" | T23: "); Serial.println(t4-t3);
//  //Serial.println("");
//  for(uint8_t i=0; i<FANARR_SIZE; ++i) {
////    Serial.print(i); Serial.print(" | ");      
////    Serial.print(times[i]); Serial.print(" | ");      
////    if (numpts[i] > 0) {
////      Serial.print(toRPM(times[i],numpts[i]));
////    } else {
////      Serial.print(0);
////    }
////    Serial.print(" | ");
////    Serial.println(numpts[i]);  
//    if (numpts[i] > 0) {
//      Serial.print(i); Serial.print(" | ");      
//      Serial.print(times[i]); Serial.print(" | ");           
//      Serial.print(fanrpms[i]); Serial.print(" | ");       
//      Serial.println(numpts[i]);  
//    }    
//  } 
//  #if HASLCD
//   GUIloop(); 
//  #endif
//  tachCleanup();  
//  Serial.println("---");
//  delay(1500);
//}
//
//
////*****************************************************************
//
//inline void setup_test_IR_PWM_TACH() {
//  setupIRSensor();
//  setupTimer1(TIMER1_DEF_FREQ);
//  setupFanController();
//  setupTach();
//  Serial.println("**********************************");
//}
//
//inline void loop_test_IR_PWM_TACH() {
//  uint32_t t1 = micros();
//  updateTemps();  
//  doFanCtrlUpdate();
//  uint32_t t2 = micros();
//  delay(500);
//  uint32_t t3 = micros();  
//  doTachRun(100000);
//  uint32_t t4 = micros();  
//  fanCtrlAlarmCheck();
//  Serial.print("IR: "); Serial.print(temp_IR);
//  Serial.print(" | SENS: "); Serial.print(temp_sensor);
//  Serial.print(" | CHIP: "); Serial.print(temp_chip);
//  Serial.print(" | T12: "); Serial.print(t2-t1);
//  Serial.print(" | T23: "); Serial.println(t4-t3);
//  //Serial.println("");
//  for(uint8_t i=0; i<FANARR_SIZE; ++i) {
////    Serial.print(i); Serial.print(" | ");      
////    Serial.print(times[i]); Serial.print(" | ");      
////    if (numpts[i] > 0) {
////      Serial.print(toRPM(times[i],numpts[i]));
////    } else {
////      Serial.print(0);
////    }
////    Serial.print(" | ");
////    Serial.println(numpts[i]);  
//    if (numpts[i] > 0) {
//      Serial.print(i); Serial.print(" | ");      
//      Serial.print(times[i]); Serial.print(" | ");           
//      Serial.print(fanrpms[i]); Serial.print(" | ");       
//      Serial.println(numpts[i]);  
//    }    
//  }
//  Serial.println("");   
//  tachCleanup();
//  delay(1500);
//}
//
////*****************************************************************
//
//inline void setup_test_IR_PWM_ADC() {
//  setupIRSensor();
//  setupTimer1(TIMER1_DEF_FREQ);
//  setupFanController();
//  setupADC();
//}
//
//inline void loop_test_IR_PWM_ADC() {
//  uint32_t t1 = micros();
//  updateTemps();  
//  doFanCtrlUpdate();
//  uint32_t t2 = micros();  
//  updatePowerData();
//  uint32_t t3 = micros();  
//  Serial.print("IR: "); Serial.print(temp_IR);
//  Serial.print(" | SENS: "); Serial.print(temp_sensor);
//  Serial.print(" | CHIP: "); Serial.print(temp_chip);
//  Serial.print(" | T12: "); Serial.print(t2-t1);
//  Serial.print(" | T23: "); Serial.println(t3-t2);
//  Serial.println("");
//  delay(500);
//}
