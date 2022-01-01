#include "constants.h"
#include "log.h"

#ifndef globals_h
#define globals_h

// General
inline volatile uint8_t systemCode = 0;       // System state (0=normal)
inline char statusString[LCD_STATUSLEN+1];
inline uint8_t lastErrorCode = 0;
inline uint32_t errCnt = 0;
inline char lastErrorString[LCD_STATUSLEN+1];
inline volatile bool systemError = false;
inline volatile uint8_t exitCode = 0;

// EEPROM
inline uint32_t lastEEPROMSaveTime;
inline uint8_t eeprom_guimode;
inline uint8_t eeprom_manstate;

// Temperature
inline float temp_IR;
inline float temp_sensor;
inline float temp_chip;

// Fan control
inline uint8_t fan_state = 0;                // 0=off, 1=min, 1-11=curve, 12=max
inline uint8_t new_state = 0;
inline volatile uint8_t man_state = 0;
inline bool spinup_required = false;
inline volatile bool manualpwm_changed = false;
inline volatile bool immediate_fan_updt_reqd = false;
inline volatile bool fan_mode_changed = false;
inline uint8_t fanFailScores[FANARR_SIZE];

// Timer
inline uint16_t t1_topcnt;
inline unsigned long t1_freq;
inline volatile uint8_t t1_pwm_a = 100;
inline volatile uint8_t t1_pwm_b = 100;

// ADC
inline uint8_t adc_state = 0;
inline float fan_voltage = 0.0;
inline float fan_current = 0.0;
inline float fan_power = 0.0;

// Tach
inline volatile bool firstINT = true;
inline uint16_t fanrpmssum[FANARR_SIZE];
inline uint16_t fanrpmsarr[FANARR_SIZE][TACH_SMOOTHNUM];
inline uint16_t fanrpms[FANARR_SIZE];
inline uint16_t fanrpmavg;
inline volatile unsigned long times[FANARR_SIZE];
inline volatile uint16_t numpts[FANARR_SIZE];

#if DEBUG
  inline volatile int8_t PCINT2_err;
  inline volatile int8_t PCINT1_err;
  inline volatile unsigned long ISRtime;
  inline volatile unsigned long ISRfirsttime;
  inline volatile uint16_t numISRcalls;
#endif

// LOOPING
inline uint32_t temp_update_time;
inline uint32_t fanctrl_update_time;
inline uint32_t tach_update_time;
inline uint32_t gui_update_time;
inline uint32_t adc_update_time;
inline uint32_t led_update_time;

// LED
inline uint8_t led_green_state;
inline uint8_t led_red_state;
inline uint8_t led_green_counter;
inline uint8_t led_red_counter;
inline volatile bool led_immediate_update = false;

// GUI
inline uint8_t GUIstate = 0;
inline volatile uint8_t GUImode = 0;           //0=normal, 1=manual
inline volatile bool GUImode_changed = false;
inline volatile bool GUI_immediate_update = false;
inline uint8_t GUIstate_old;
inline uint8_t GUIstate_toplim;
inline uint8_t GUIstate_botlim;
inline uint8_t LCDstate;
inline byte LCD_character_array[8];
inline uint16_t LCD_update_period;

inline uint32_t GUItime_old = 0;
inline volatile bool GUI_update_required;

inline uint8_t GUIrotation;
inline volatile uint8_t GUIrotation_skipnext;
inline uint8_t GUItimeout;

inline volatile uint32_t lastButtonTime;
inline volatile uint32_t lastEncoderTime;

inline volatile bool button_down;
inline volatile bool encoder_down;

//const char pad[500] PROGMEM = { 0 };

inline Logger logger;

// extern SoftWire wire;


#endif