static void setupADC() {
  #if (DEBUG)
    Serial.println("Setting up ADC");
  #endif
  noInterrupts();
  ADCSRA &= ~_BV(ADEN);                               // Disable the ADC
  ADMUX   = _BV(REFS0) | 0x0F;                        // Select AVcc reference, ground channel
  ADCSRA &= ~(_BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2));  // Clear prescaler bits
//  ADCSRA |= _BV(ADPS0) | _BV(ADPS1) | _BV(ADPS2);     // 128
//  ADCSRA |= _BV(ADPS0) | _BV(ADPS2);                  // 32 
  ADCSRA |= _BV(ADPS1) | _BV(ADPS2);                  // Prescaler = 64
  ADCSRA &= ~(_BV(ADATE) | _BV(ADIE));                // Disable auto trigger and interrupt
  ADCSRA |= _BV(ADIF);                                // Clear interrupt flag
  ADCSRB  = 0x00;                                     // Disable other stuff
  ADCSRA |= _BV(ADEN);                                // Enable the ADC (to set above settings)
  ADCSRA &= ~_BV(ADEN);                               // Disable the ADC
  interrupts();
}

static void updatePowerData() {
  if (!adc_state) {
    enableADC();
  }
  #if FAKE_ADC_OUTPUT
    #if (DEBUG)
      Serial.println("Running ADC update (fake)");
    #endif    
    setADCPin(ADC_VPIN);
    uint16_t V = 485;   
    setADCPin(ADC_IPIN);    
    uint16_t I = 403;
  #else
    #if (DEBUG)
      Serial.println("Running ADC update");
    #endif 
    setADCPin(ADC_VPIN);
    uint16_t V = getADCValue();
    setADCPin(ADC_IPIN);
    uint16_t I = getADCValue();   
  #endif
  fan_voltage = (V*25)/1023.0;
  fan_current = (I*5)/1023.0;
  fan_power = fan_voltage*fan_current;
  #if (DEBUG > 1)
    Serial.print("Fan power: V="); Serial.print(fan_voltage); bl(); Serial.print(V); br();
    Serial.print(" | I="); Serial.print(fan_current); bl(); Serial.print(I); br();
    Serial.print(" | P="); Serial.println(fan_power);
  #endif
  setADCPin(0xFF);
}

static inline void enableADC() {
  #if (DEBUG > 2)
    Serial.println("Enabling ADC");
  #endif
  noInterrupts();
  ADCSRA |= _BV(ADEN);                                // Enable the ADC  
  adc_state = 1;
  interrupts();
}

static inline void disableADC() {
  #if (DEBUG > 2)
    Serial.println("Disabling ADC");
  #endif
  noInterrupts();
  ADMUX   = _BV(REFS0) | 0x0F;                        // Select AVcc reference, ground channel
  ADCSRA &= ~(_BV(ADATE) | _BV(ADIE));                // Disable auto trigger and interrupt
  ADCSRA |= _BV(ADIF);                                // Clear interrupt flag
  ADCSRA &= ~(_BV(ADEN));                             // Disable the ADC
  adc_state = 0;
  interrupts();
}

static double getChipTemperature() {
  // Internal temperature has to be used with the internal reference of 1.1V
  ADCSRA &= ~_BV(ADEN);                               // Disable the ADC
  ADMUX   = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);      // Select 1.1V internal reference and temperature channel
  ADCSRA &= ~(_BV(ADATE) | _BV(ADIE));                // Disable auto trigger and interrupt
  ADCSRA |= _BV(ADIF);                                // Clear interrupt flag

  ADCSRA |= _BV(ADEN);                   // Enable the ADC
  delay(5);                              // Internal bandgap reference needs time to become stable
  ADCSRA |= _BV(ADSC);                   // Start the ADC  
  while (bit_is_set(ADCSRA,ADSC));       // Detect end-of-conversion 
  ADCSRA |= _BV(ADSC);                   // Start second run (for accuracy)
  while (bit_is_set(ADCSRA,ADSC));       // Detect end-of-conversion  
  uint16_t wADC = ADCW;                  // Reading 'register' "ADCW" takes care of how to read ADCL and ADCH 
  double t = (wADC - 324.31 ) / 1.22;    // Offset of 324.31 could be wrong, just an indication 
  setADCPin(0xFF);                       // Set ADC back to external source
  return t;                              // The returned temperature is in degrees Celsius
}

static uint16_t getADCValue() {
  if (bit_is_set(ADCSRA,ADEN)) {
    #if (DEBUG > 2)
      Serial.println(F("Running ADC conversion"));
    #endif
    //while (bit_is_set(ADCSRA,ADSC));  // If conversion in progress, wait
    ADCSRA |= _BV(ADSC);                // Start the ADC
    while (bit_is_set(ADCSRA,ADSC));    // Detect end-of-conversion 
    ADCSRA |= _BV(ADSC);                // Start the ADC
    while (bit_is_set(ADCSRA,ADSC));    // Detect end-of-conversion 
    return ADCW;
  } else {
    return 0;
  }
}

static void adcAlarmCheck() {
  #if (DEBUG > 2)
    Serial.println(F("ADC alarm check"));
  #endif
  if (fan_voltage < ADC_V_LOWALARM) {
    declareError(ADC_VTOOLOW);
  } else if (fan_voltage > ADC_V_HIGHALARM) {
    declareError(ADC_VTOOHIGH);
  } else if (fan_current > ADC_I_ALARM) {
    declareError(ADC_ITOOHIGH);
  }
}

static void setADCPin(uint8_t pin) {
  #if (DEBUG > 2)
    Serial.print(F("ADC pin set to ")); Serial.println(pin); 
  #endif
  if (pin <= 7) {
    ADMUX = _BV(REFS0) | (pin & 0x07);                // Select AVcc reference+set pin select
  } else {
    ADMUX = _BV(REFS0) | 0x0F;                        // AVcc + ground measurement
  }
  //  uint8_t pinselect;
//  switch(pin) {
//    case 0:
//      pinselect = 0x00;
//      break;
//    case 1:
//      pinselect = _BV(MUX0);
//      break;
//    case 2:
//      pinselect = _BV(MUX1);
//      break;
//    case 3:
//      pinselect = _BV(MUX1) | _BV(MUX0);
//      break;
//    case 4:
//      pinselect = _BV(MUX2);
//      break;
//    case 5:
//      pinselect = _BV(MUX2) | _BV(MUX0);
//      break;
//    default:
//      pinselect = 0x0F; //Ground
//      break;      
//  }
//  ADMUX |= pinselect;
}
