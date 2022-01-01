#include "led.h"

void setupLED() {
  #if (DEBUG>1)
    Serial.println("Setting up LED");
  #endif
  LED_GREEN_PORT &= ~(LED_GREEN_PIN);
  LED_GREEN_DDR |= LED_GREEN_PIN;
  LED_RED_PORT &= ~(LED_RED_PIN);
  LED_RED_DDR |= LED_RED_PIN;
  led_green_state = led_red_state = 0;
}

void setOKLed(uint8_t state) {
  #if (LED_DEBUG>2)
    Serial.print(" LED green set: "); Serial.println(state);
  #endif 
  if (led_green_state != state) {
    led_green_state = state;
    switch(state) {
      case (LED_OFF):
        LED_GREEN_PORT &= ~LED_GREEN_PIN;
        break;
      case (LED_ON): 
        LED_GREEN_PORT |= LED_GREEN_PIN;
        break;
      case (LED_BLINK_SLOW):
        led_green_counter = 0;
        break;
      case (LED_FLASH):
        led_green_counter = 0;
        break;
      case LED_FAST_FLASH:
        led_green_counter = 0;
        LED_GREEN_PORT &= ~LED_GREEN_PIN;
        break;
      default:
        LED_GREEN_PORT &= ~LED_GREEN_PIN;
        break;
    }
    queueLedUpdate();
  }  
}

void setErrLed(uint8_t state) {
  #if (LED_DEBUG>1)
    Serial.print(" LED red set: "); Serial.println(state);
  #endif 
  if (led_red_state != state) {
    led_red_state = state;
    switch(state) {
      case(0):
        // fall through
      case(1):
        LED_RED_PORT &= ~LED_RED_PIN;
        break;
      default:
        LED_RED_PORT |= LED_RED_PIN;
        break;
    }
    queueLedUpdate();
  }
}

void queueLedUpdate() {
  // Queue LED update by zeroing out the next update time
  led_immediate_update = true;
}

void updateLED() {
  #if (LED_DEBUG>2)
    Serial.print(" UpdateLED: "); Serial.print(led_green_state); Serial.print(led_red_state);
    Serial.print(fan_state); Serial.print(GUImode);
    Serial.print(led_green_counter); Serial.println(led_red_counter);
  #endif 

  switch(led_green_state) {
    case(LED_BLINK_SLOW):
      if (led_green_counter > 0) {
        --led_green_counter;
      } else {
        LED_GREEN_PORT ^= LED_GREEN_PIN;
        led_green_counter = (LED_BLINK_PERIOD/LOOP_LED_UPDT)/2;
      }
      break;
    case(LED_FLASH):
      if (led_green_counter > 0) {
        --led_green_counter;
      } else {
        // Set new timeout
        if (LED_GREEN_PORT & LED_GREEN_PIN) {
          // Long section with led off
          led_green_counter = 100;
        } else {
          // Short section with led on
          led_green_counter = 1;
        }  
        // Toggle led 
        LED_GREEN_PORT ^= LED_GREEN_PIN;     
      }
      break;
    case(LED_FAST_FLASH):
      if (led_green_counter > 0) {
        --led_green_counter;
      } else {
        // Blink LED on
        led_green_counter = LED_FAST_FLASH_N; 
        LED_GREEN_PORT |= LED_GREEN_PIN;
        delayMicroseconds(2000);
        LED_GREEN_PORT &= ~LED_GREEN_PIN;                 
      }
      break;
    default:
      break;
  }
//  if (led_green_state == 1) {
//    if (green_blink > (LED_BLINK_PERIOD/LOOP_LED_UPDT)/2) {
//      LED_GREEN_PORT ^= LED_GREEN_PIN;
//      green_blink = 0;
//    } else {
//      ++green_blink;
//      //activity = true;
//    }
//  } else 
//  if (led_green_state == LED_BLINK_SLOW && GUImode == GUI_AUTO) {
//    if (fan_state == 0) {
//      setOKLed(LED_BLINK_SLOW);
//    }
//  } else if (led_green_state == LED_FLASH) {
//    if (fan_state > 0) {
//      // Fan on
//      setOKLed(2);
//    } else {
//      // Blink for 1/30 of usual period
//      //if (green_blink > (LED_BLINK_PERIOD/LOOP_LED_UPDT)/2) {
//      // Blink for 1 cycle
//      if (green_blink > 0) {
//        LED_GREEN_PORT &= ~LED_GREEN_PIN;
//      } else {
//        LED_GREEN_PORT |= LED_GREEN_PIN;
//      }
//      ++green_blink;
//      if (green_blink > 30*(LED_BLINK_PERIOD/LOOP_LED_UPDT)) {
//        green_blink = 0;     
//      }
//    }
//  }
  
  if (led_red_state == 1) {
    if (led_red_counter > LED_BLINK_PERIOD/LOOP_LED_UPDT) {
      LED_RED_PORT ^= LED_RED_PIN;
      led_red_counter = 0;
    } else {
      ++led_red_counter;
    }
  }
}
