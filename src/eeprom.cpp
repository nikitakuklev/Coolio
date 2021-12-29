// byte 0 = sig
// byte 1 = mode
// byte 2 = manual fan state

static void restoreEEPROMvals() {
  #if (DEBUG)
     Serial.println("Reading EEPROM");
  #endif
  if (eeprom_read_byte((uint8_t*)0) == EEPROM_SIG) {
    // Restore mode and manual state
    eeprom_guimode = GUImode = eeprom_read_byte((uint8_t*)1);
    eeprom_manstate = man_state = eeprom_read_byte((uint8_t*)2);
    
    fan_state = 0;
    new_state = man_state;
    if (GUImode) {
      manualpwm_changed = true;
    }
    #if (DEBUG>2)
     Serial.print(F("EEPROM sig OK:")); Serial.print(GUImode); Serial.print("|"); Serial.println(man_state);     
    #endif
  } else {
    // Clear relevant bytes
    #if (DEBUG>2)
     Serial.println(F("EEPROM mistmatch!"));
    #endif
    eeprom_update_byte((uint8_t*)0, EEPROM_SIG);
    eeprom_update_byte((uint8_t*)1, 0);
    eeprom_update_byte((uint8_t*)2, 0);
  }
  lastEEPROMSaveTime = millis();
}

static void updateEEPROMvals() {
  #if (DEBUG)
     Serial.println(F("Saving EEPROM:")); Serial.print(GUImode); Serial.print("|"); Serial.println(man_state); 
  #endif
  eeprom_guimode = GUImode;
  eeprom_update_byte((uint8_t*)1, GUImode);
  eeprom_manstate = man_state;
  eeprom_update_byte((uint8_t*)2, man_state);  
  lastEEPROMSaveTime = millis();
}
