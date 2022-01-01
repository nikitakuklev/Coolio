#include "globals.h"
#include "extras.h"
#include "timer.h"

void setupTach() {
  #if (DEBUG)
    Serial.println(F("Setting up tach"));
  #endif
  noInterrupts();  
  DDRD &= ~TACH_PORTD_MASK;             // Set port D to input 
  PORTD |= TACH_PORTD_MASK;             // Set port D pullups 
  PCMSK2 = TACH_PORTD_MASK;             // Setup port D mask
  DDRC &= ~TACH_PORTC_MASK;             // Set port C to input
  PORTC |= TACH_PORTC_MASK;             // Set port C pullups 
  PCMSK1 = TACH_PORTC_MASK;             // Setup port C mask 
  interrupts(); 
  #if (DEBUG > 2)  
    printT1Registers(); printPCRegisters(); printPORTDRegisters(); printPORTCRegisters();
  #endif
}

void doTachRun(unsigned long timeout) {
  if (fan_state == 0) {
    
  }
  //stop PWM and bring pins high 
  noInterrupts();
  stopTimer1(); 
  setTimerPinsHigh();
  
  #if (DEBUG > 2)
    interrupts(); 
    Serial.print(F(" Starting tach for (us): ")); Serial.println(timeout);
    printT1Registers(); printPCRegisters(); printPORTDRegisters(); printPORTCRegisters();
    noInterrupts();
    PCINT2_err = 0; PCINT1_err = 0;
  #endif
  
  // declare flag...not too efficient, but need a variable outside ISR for first run
  firstINT = true;  
  uint32_t startTime = micros();  
  PCIFR |= (_BV(PCIF2)|_BV(PCIF1));          // Clear interrupt flag 
  PCICR |= (_BV(PCIE2)|_BV(PCIE1));          // Enable PC interrupt  
  interrupts();
  //do {} while (micros() - startTime < timeout);
  for(uint8_t flag = 1;flag;) {
    noInterrupts();
    flag = (micros() - startTime) < timeout;
    interrupts();  
  }
  noInterrupts();  
  PCICR &= ~(_BV(PCIE2)|_BV(PCIE1));         // Disable PC interrupt    
  resumeTimer1();
  interrupts();
  
  #if (DEBUG > 1)    
    Serial.print(F(" ErrD: ")); Serial.print(PCINT2_err);
    Serial.print(F(" | ErrC: ")); Serial.println(PCINT1_err);
    uint16_t sum = 0;
    for (uint8_t i=0; i<FANARR_SIZE; i++) {
      if (numpts[i] > 0) {
        sum += numpts[i]+1;
      }
    }
    Serial.print(F(" First ISR:")); Serial.print(ISRfirsttime);
    Serial.print(F(" | Others (t|#|avg): ")); Serial.print(ISRtime); 
    Serial.print(" | "); Serial.print(numISRcalls);
    Serial.print(" | "); Serial.print(ISRtime/numISRcalls);
    Serial.print(F(" | Missed:")); Serial.println(numISRcalls-sum);
  #endif
  
  // rolling average via rotating buffer, so to speak
  static uint8_t readIndex = 0;
  uint16_t fanavg = 0;
  for (uint8_t i=0; i<FANARR_SIZE; i++) {
    if (numpts[i] > 0) {
      fanrpmssum[i] = fanrpmssum[i]-fanrpmsarr[i][readIndex]+toRPM(times[i],numpts[i]);
      fanrpmsarr[i][readIndex] = toRPM(times[i],numpts[i]);
      fanrpms[i] = fanrpmssum[i]/TACH_SMOOTHNUM;
      fanavg += fanrpms[i];    
    } else {
      fanrpmssum[i] = fanrpmssum[i]-fanrpmsarr[i][readIndex];
      fanrpmsarr[i][readIndex] = 0;
      fanrpms[i] = fanrpmssum[i]/TACH_SMOOTHNUM;
    }
  }
  readIndex++;
  if (readIndex >= TACH_SMOOTHNUM) {
    readIndex = 0;
  }
  fanrpmavg = fanavg/FANCTRL_ACTFANSN;
//  #if (DEBUG>1) 
//    printFanDataMore();
//  #endif  
}

// This ISR handles in fastest way the update of fan tach times
// Note that even though its possible to deal with more than 1 bit changing,
// we don't...tests show that these are veeeery rare (6 fans@600rpm = 1miss/5s)
#define PORTSZ  PORTD_SIZE
#define PORTSFT PORTD_SHIFT
#define PORTFP  PORTD_FIRSTPIN
#define PIN     PIND
#define ERRVAR  PCINT2_err
#define NUMISR  numISRcalls
ISR (PCINT2_vect) {
  static uint8_t lastPIN;
  static uint32_t lastTrigTime[PORTSZ];  
  #if (DEBUG>1)
    uint32_t time1 = micros();
  #endif
  
  if (firstINT) {
    // First time we just capture port state 
    lastPIN = PIN;
    firstINT = false;
    memset(lastTrigTime, 0, sizeof(uint32_t)*PORTSZ);    
    #if (DEBUG>1)
      ISRfirsttime = micros()-time1;
    #endif
  } else { 
    // Now, find which one flipped and update respective index in arrays
    // Different bits will be 1  
    uint8_t PINtemp = PIN;  
    uint8_t diff = (PINtemp ^ lastPIN);
    lastPIN = PINtemp;
    uint32_t curTime = micros();
    diff >>= PORTFP; 
    
    #if (DEBUG)
      if (!diff) {
        ERRVAR = -1;
        return;
      }
    #endif   
    
    // Checks if more than 1 bit changed (means will miss at least 1 event)
    #if (DEBUG)
      uint8_t temp = 0;
      uint8_t diff2 = diff;
      //skip first two bits
      for (;diff2 >>= 1;) {
        if(diff2 & 0x01) {
          ++temp;
        }
      }
      if (ERRVAR < temp) {
        ERRVAR = temp;
      }
    #endif

    // Inefficient version of clz...grrrrrrrrrrr
    uint8_t r = 0;
    for (;diff >>= 1;++r) { }

    // If we already have nonzero time, find delta and export to volatile
    // Else, init with first measurement
    if (lastTrigTime[r]) {
      times[r+PORTSFT] += (curTime - lastTrigTime[r]);
      ++numpts[r+PORTSFT];
      lastTrigTime[r] = curTime;
    } else {
      lastTrigTime[r] = curTime;
    }
    
    #if (DEBUG>1)
      ISRtime += micros()-time1;
      ++numISRcalls;
    #endif
  }
}

#undef PORTSZ
#undef PORTSFT
#undef PORTFP 
#undef PIN 
#undef ERRVAR
#define PORTSZ  PORTC_SIZE
#define PORTSFT PORTC_SHIFT
#define PORTFP  PORTC_FIRSTPIN
#define PIN     PINC
#define ERRVAR  PCINT1_err
ISR (PCINT1_vect) {
  static uint8_t lastPIN;
  static uint32_t lastTrigTime[PORTSZ];  
  #if (DEBUG>1)
    uint32_t time1 = micros();
  #endif
  
  if (firstINT) {
    // First time we just capture port state 
    lastPIN = PIN;
    firstINT = false;
    memset(lastTrigTime, 0, sizeof(uint32_t)*PORTSZ);    
    #if (DEBUG>1)
      ISRfirsttime = micros()-time1;
    #endif
  } else { 
    // Now, find which one flipped and update respective index in arrays
    // Different bits will be 1  
    uint8_t PINtemp = PIN;  
    uint8_t diff = (PINtemp ^ lastPIN);
    lastPIN = PINtemp;
    uint32_t curTime = micros();
    diff >>= PORTFP; 
    
    #if DEBUG
      if (!diff) {
        ERRVAR = -1;
        return;
      }
    #endif   
    
    // Checks if more than 1 bit changed (means will miss at least 1 event)
    #if DEBUG
      uint8_t temp = 0;
      uint8_t diff2 = diff;
      //skip first two bits
      for (;diff2 >>= 1;) {
        if(diff2 & 0x01) {
          ++temp;
        }
      }
      if (ERRVAR < temp) {
        ERRVAR = temp;
      }
    #endif

    // Inefficient version of clz...grrrrrrrrrrr
    uint8_t r = 0;
    for (;diff >>= 1;++r) { }

    // If we already have nonzero time, find delta and export to volatile
    // Else, init with first measurement
    if (lastTrigTime[r]) {
      times[r+PORTSFT] += (curTime - lastTrigTime[r]);
      ++numpts[r+PORTSFT];
      lastTrigTime[r] = curTime;
    } else {
      lastTrigTime[r] = curTime;
    }
    
    #if DEBUG>1
      ISRtime += micros()-time1;
      ++numISRcalls;
    #endif
  }
}

#undef PORTSZ
#undef PORTSFT
#undef PORTFP 
#undef PIN 
#undef ERRVAR

inline void tachCleanup() {
  // We need to clean up volatiles, but can't use memset because 
  // "hurr durr C++ undefined behaviour casting away volatile modifier"
  // so need to write our own loop and hope it gets optimized 
  for(uint8_t i=0; i<FANARR_SIZE; ++i) {
    times[i] = 0; numpts[i] = 0;      
  }    
  #if (DEBUG)
    numISRcalls = 0; 
    PCINT2_err = PCINT1_err = 0;
    ISRtime = ISRfirsttime = 0;
  #endif
}


