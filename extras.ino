#if (DEBUG)

static void printBits(byte myByte){
  for(byte mask = 0x80; mask; mask >>= 1){
    if(mask  & myByte) {
       Serial.print('1');
    } else {
       Serial.print('0');
    }
  }
}

static void printT1Registers() {
  #if (DEBUG > 3)
    Serial.print("T1 | TCCR1A:"); printBits(TCCR1A); 
    Serial.print(" | TCCR1B:"); printBits(TCCR1B);
    Serial.print(" | DDRB:  "); printBits(DDRB);
    Serial.print(" | PORTB: "); printBits(PORTB); 
    Serial.println("");
  #endif
}

static void printPORTDRegisters() {
  #if (DEBUG > 3)
    Serial.print("PD | DDRD:  "); printBits(DDRD); 
    Serial.print(" | PORTD: "); printBits(PORTD);
    Serial.print(" | PIND:  "); printBits(PIND);
    Serial.println("");
  #endif
}

static void printPORTCRegisters() {
  #if (DEBUG > 3)
  Serial.print("PC | DDRC:  "); printBits(DDRC); 
  Serial.print(" | PORTC: "); printBits(PORTC);
  Serial.print(" | PINC:  "); printBits(PINC);
  Serial.println("");
  #endif
}

static void printPCRegisters() {
  #if (DEBUG > 3)
  Serial.print("PC | PCIFR: "); printBits(PCIFR); 
  Serial.print(" | PCMSK2:"); printBits(PCMSK2);
  Serial.print(" | PCMSK1:"); printBits(PCMSK1);
  Serial.print(" | PCICR: "); printBits(PCICR); 
  Serial.println("");
  #endif
}

//static void printFanData(uint8_t fnum, unsigned long rot) {
//  Serial.print("Fan "); Serial.print(fnum); Serial.print(" - ");
//  Serial.print(rot);
//  Serial.print(" | Fan:");
//  Serial.println(60000000/(rot*4));
//}

static void printFanDataMore() {
    for(uint8_t i=0; i<FANARR_SIZE; ++i) {
//    Serial.print(i); Serial.print(" | ");      
//    Serial.print(times[i]); Serial.print(" | ");      
//    if (numpts[i] > 0) {
//      Serial.print(toRPM(times[i],numpts[i]));
//    } else {
//      Serial.print(0);
//    }
//    Serial.print(" | ");
//    Serial.println(numpts[i]);  
    if (numpts[i] > 0) {
      Serial.print(i); Serial.print(" | ");      
      Serial.print(times[i]); Serial.print(" | ");           
      Serial.print(fanrpms[i]); Serial.print(" | ");       
      Serial.println(numpts[i]);  
    }         
  }
  Serial.print("Avg: "); Serial.println(fanrpmavg);
}

static void printTemps() {
  Serial.print("IR: "); Serial.print(temp_IR);
  Serial.print(" | SENS: "); Serial.print(temp_sensor);
  Serial.print(" | CHIP: "); Serial.println(temp_chip);
}

static inline void bl() {
  Serial.print("(");
}
static inline void br() {
  Serial.print(")");
}

#endif

static inline uint16_t toRPM(uint32_t deltat, uint16_t num) {
  if (num == 0) {
    return 0;
  } else {
    return (uint16_t)((1500000000UL/(deltat))*((long)num)/100);
  }  
}
