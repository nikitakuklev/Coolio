static void setupLED() {
  #if (DEBUG>1)
    Serial.println("Setting up LED");
  #endif
  LED_GREEN_PORT &= ~(LED_GREEN_PIN);
  LED_GREEN_DDR |= LED_GREEN_PIN;
  LED_RED_PORT &= ~(LED_RED_PIN);
  LED_RED_DDR |= LED_RED_PIN;
  led_green_state = led_red_state = 0;
}

static void setLED_normal() {
  if (fan_state == 0) {
    setOKLed(3);
  } else {
    setOKLed(2);
  }
  if (lastErrorCode == 0 && systemCode == 0) {
    setErrLed(0);
  } else {
    setErrLed(1);
  }
}

static void setLED_nonfatalerr() {
  setErrLed(1);
}

static void setLED_manualmode() {
  setOKLed(1);
  if (lastErrorCode == 0 && systemCode == 0) {
    setErrLed(0);
  } else {
    setErrLed(1);
  }
  
}

static void setLED_panic() {
  setOKLed(0);
  setErrLed(2);
}

static void setOKLed(uint8_t state) {
  #if (DEBUG>2)
    Serial.print(" LED green to "); Serial.println(state);
  #endif 
  if (led_green_state != state) {
    led_green_state = state;
    //green_blink = -1;
    switch(state) {
      case (LED_OFF):
        LED_GREEN_PORT &= ~(LED_GREEN_PIN);
        break;
      case (LED_ON): 
        LED_GREEN_PORT |= LED_GREEN_PIN;
        break;
      case (LED_BLINK_SLOW):
        green_blink = (LED_BLINK_PERIOD/LOOP_LED_UPDT)/2;
        break;
      case (LED_FLASH):
        green_blink = 1;
        break;
      default:
        LED_GREEN_PORT &= ~(LED_GREEN_PIN);
        break;
    }
    updateLED();
  }  
}

static void setErrLed(uint8_t state) {
  #if (DEBUG>1)
    Serial.print(" LED red to "); Serial.println(state);
  #endif 
  if (led_red_state != state) {
    led_red_state = state;
    switch(state) {
      case(0):
        // fall through
      case(1):
        LED_RED_PORT &= ~(LED_RED_PIN);
        break;
      default:
        LED_RED_PORT |= LED_RED_PIN;
        break;
    }
  }
}

static void updateLED() {
  static uint8_t green_blink = 0;
  static uint8_t red_blink = 0;
  #if (DEBUG>2)
    Serial.print(" Updating LEDs: "); Serial.print(led_green_state); Serial.print(led_red_state);
    Serial.print(fan_state); Serial.print(GUImode);
    Serial.print(green_blink); Serial.println(red_blink);
  #endif 
//  if (led_green_state == 1 && led_red_state == 1) {
//    
//  }
  switch(led_green_state) {
    case(LED_BLINK):
      if (green_blink > 0) {
        --green_blink;
      } else {
        LED_GREEN_PORT ^= LED_GREEN_PIN;
        green_blink = (LED_BLINK_PERIOD/LOOP_LED_UPDT)/2;
      }
      break;
    case(LED_FLASH):
      if (green_blink > 0) {
        --green_blink;
      } else {
        LED_GREEN_PORT ^= LED_GREEN_PIN;
        green_blink = 1;
      }
      break;
    default:
      break;
  }
  if (led_green_state == 1) {
    if (green_blink > (LED_BLINK_PERIOD/LOOP_LED_UPDT)/2) {
      LED_GREEN_PORT ^= LED_GREEN_PIN;
      green_blink = 0;
    } else {
      ++green_blink;
      //activity = true;
    }
  } else if (led_green_state == 2 && GUImode == 0) {
    if (fan_state == 0) {
      setOKLed(3);
    }
  } else if (led_green_state == 3) {
    if (fan_state > 0) {
      setOKLed(2);
    } else {
      // Blink for 1/30 of usual period
      //if (green_blink > (LED_BLINK_PERIOD/LOOP_LED_UPDT)/2) {
      // Blink for 1 cycle
      if (green_blink > 0) {
        LED_GREEN_PORT &= ~LED_GREEN_PIN;
      } else {
        LED_GREEN_PORT |= LED_GREEN_PIN;
      }
      ++green_blink;
      if (green_blink > 30*(LED_BLINK_PERIOD/LOOP_LED_UPDT)) {
        green_blink = 0;     
      }
    }
  }
  
  if (led_red_state == 1) {
    if (red_blink > LED_BLINK_PERIOD/LOOP_LED_UPDT) {
      LED_RED_PORT ^= LED_RED_PIN;
      red_blink = 0;
    } else {
      ++red_blink;
    }
  }
}
