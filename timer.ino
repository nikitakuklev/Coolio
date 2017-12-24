static void analogWriteT1Raw(uint8_t pin, uint16_t value) {
  // Note that writing to TCNT1 blocks compare on next cycle,
  // which means will miss if value=0 unless we do special case
  // (Not relevant for now)
  if (value > t1_topcnt) {
    declareError(TIMER1_WRONG_PWM);
    #if (DEBUG)
      Serial.print("Timer 1 raw: WRONG VALUE "); Serial.println(value); 
    #endif
  } else {
    switch (pin) {
      case TIMER1_A_PIN:
          OCR1A = value;
          break;
      case TIMER1_B_PIN:
          OCR1B = value;
          break;
      default:        
          declareError(TIMER1_WRONG_PIN);
          #if (DEBUG)
            Serial.print("Timer 1 raw: WRONG PIN "); Serial.println(pin); 
          #endif
          break; // no other pin will work
    }
    #if (DEBUG > 1)
      Serial.print("Timer 1 raw: pin "); Serial.print(pin);
      Serial.print(" set to value "); Serial.println(value);
    #endif
  }
}

static void analogWriteT1(uint8_t pin, uint32_t value) {
  if (value > 100) {
    declareError(TIMER1_WRONG_PWM);
    #if (DEBUG)
      Serial.print("Timer 1: WRONG VALUE"); Serial.println(value); 
    #endif
  } else {   
    #if (DEBUG)
      Serial.print("Timer 1 passing to raw: "); Serial.println(value); 
    #endif
    value = (uint16_t)((value*t1_topcnt)/100);
    analogWriteT1Raw(pin,value);
//    switch (pin) {
//      case TIMER1_A_PIN:
//          OCR1A = value;
//          break;
//      case TIMER1_B_PIN:
//          OCR1B = value;
//          break;
//      default:      
//          declareError(TIMER1_WRONG_PIN); 
//          #if (DEBUG)
//            Serial.print("Timer 1 raw: WRONG PIN "); Serial.println(pin); 
//          #endif
//          break; // no other pin will work
//    }
//    #if (DEBUG > 1)
//      Serial.print("Timer 1: pin "); Serial.print(pin);
//      Serial.print(" set to value "); Serial.println(value);
//    #endif
  }
}

/*
Starts timer 1 with the specified frequency under 31kHz
*/
static void setupTimer1(uint16_t freq) {
  if (freq > 31000U) { declareError(TIMER1_FREQ_ERR); return; }
  noInterrupts();
  setTimerPinsHigh();
  t1_freq = freq;
  unsigned long top = F_CPU / (2*freq);  
  TCCR1A = TCCR1B = 0;        // clear control register A,B
  TIMSK1 = 0;                 // clear timer interrupt mask  
  TCNT1 = 0;                  // clear timer counter
  OCR1A = OCR1B = 0;          // reset compares  
    
  TIFR1 &= ~(_BV(OCF1A)|_BV(OCF1B)|_BV(TOV1)|_BV(ICF1)); // Clear timer interrupt flags
  //OCR1A = 31250;            // compare match register 16MHz/256/2Hz
  //TCCR1B |= _BV(WGM12);     // CTC mode

  TCCR1A = _BV(COM1A1)     // non-inverted PWM on ch. A
         | _BV(COM1B1);    // same on ch; B
//         | _BV(WGM11);   // mode 10: ph. correct PWM, TOP = ICR1         
  TCCR1B = _BV(WGM13)      // mode 8: ph. freq. correct PWM, TOP = ICR1
         | _BV(CS10)       // system clock, prescaler = 1
         | _BV(ICNC1);     // noise canceller on just in case
         
  // Note that ICR is not double buffered which can cause problems
  // but we need 2 channels and frequency is likely constant so w/e
  ICR1 = top;              // TOP value to count to before reversing, typically 320 (16mhz/25khz/2)
  t1_topcnt = top;
  interrupts();   
  //TIMSK1 |= _BV(OCIE1A);    // enable timer compare interrupt
  #if DEBUG
    Serial.print("Timer 1 start: TOP="); Serial.print(top);
    Serial.print(" | freq="); Serial.println(freq);
  #endif
}

static inline void setTimerPinsHigh() {
  TIMER1_PORT |= (TIMER1_A_PIN | TIMER1_B_PIN);  // Set high
  TIMER1_DDR  |= (TIMER1_A_PIN | TIMER1_B_PIN);  // Timer pins to outputs 
}

static inline void stopTimer1() {
  // Stop PWM
  #if (DEBUG > 2)
    Serial.println("Stopping timer 1");
  #endif
  TCCR1A &= ~(_BV(COM1A1)|_BV(COM1B1)); 
}

static inline void resumeTimer1() {
  // Resume PWM
  #if (DEBUG > 2)
    Serial.println("Resuming timer 1");
  #endif
  TCCR1A |= _BV(COM1A1)|_BV(COM1B1);
}
