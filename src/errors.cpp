#include "errors.h"
#include "led.h"
#include "timer.h"
#include "adc.h"
#include "encoder.h"
#include "logic.h"
#include "lcd.h"
#include "fanctrl.h"

void declareError(uint8_t code) {
  systemCode = code;
  errCnt++;
  if (!systemError) {
    systemError = true;
  } else {
    #if (DEBUG)
      Serial.println(F("!!!DOUBLE ERROR CONDITION!!!"));      
    #endif
    #if FATAL_DOUBLEERREN
      panic(FATAL_DOUBLERR);
    #endif
  }
  updateStatusString();
  //systemCode = code;
  #if (DEBUG)
    Serial.print("ERR "); Serial.print(systemCode,HEX); 
    Serial.print(": "); Serial.println(statusString);   
  #endif
  if (isFatal(code)) {
    #if (DEBUG)
      Serial.println(F("!!!FATAL ERROR CONDITION!!!"));      
    #endif
    panic(code);
  }
  clearError();
}

bool isFatal(uint8_t code) {
  // Numeric second hex means fatal
  return (code & 0x0F) < 0x0A;  
}

void clearError() {
  setLED_nonfatalerr();
  lastErrorCode = systemCode;
  memset(lastErrorString,0,LCD_STATUSLEN*sizeof(char));
  memcpy(lastErrorString,statusString,LCD_STATUSLEN*sizeof(char));
  systemCode = 0x00;
  systemError = false;
  updateStatusString();
  #if (DEBUG)
    Serial.print("LE set to "); Serial.println(lastErrorCode, HEX);
    Serial.println("ERR cleared");
  #endif
}

void updateStatusString() {
  memset(statusString,0,sizeof(char)*LCD_STATUSLEN);  
  // Indexing by high byte of error code, and loading strings for code memory
  strcpy_P(statusString, (char*)pgm_read_word(&statusStringTable[systemCode>>4]));
  //strcpy_P(statusString, statusStringTable[systemCode>>4]);
  #if (DEBUG>2)
    Serial.print(F("Updating status string: ")); Serial.println(statusString); 
  #endif
}

void panic(uint8_t err) { 
  #if (DEBUG)
    Serial.println(F("PANIC TRIGGERED, HALTING"));
    Serial.print("Error code: "); Serial.println(err,HEX);
    delay(500);
  #endif 
  noInterrupts();
  exitCode = err;
  enableFanPower();
  disableADC();
  stopTimer1();
  TIMER1_DDR  |= TIMER1_A_PIN | TIMER1_B_PIN;   // Fans pins to max
  TIMER1_PORT |= TIMER1_A_PIN | TIMER1_B_PIN;
  disableEncoderEXTINT0();
  disableButtonEXTINT1();
  setLED_panic();
  #if HASLCD
    drawFatalErrorScreen();
  #endif
  interrupts();
  #if (DEBUG)
    Serial.println(F("GOODBYE...")); 
  #endif
  delay(500);
  // Shutdown with no interrupts = permanently
  noInterrupts();
  for(;;) { ; }
//  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  
//  MCUCR = bit (BODS) | bit (BODSE);  // turn on brown-out enable select
//  MCUCR = bit (BODS);                // this must be done within 4 clock cycles of above
//  sleep_cpu();                      // sleep within 3 clock cycles of above
}
