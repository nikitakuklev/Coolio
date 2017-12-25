#ifndef constants_h
#define constants_h
#include <avr/boot.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <inttypes.h>
#include "Arduino.h"

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 3
// 1 - general debug
// 2 - trace
// 3 - trace more
// 4 - kill me

// MACROS
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

// LOOP TIMINGS
// I am using prime times to spread out routines, increasing responsiveness
#define LOOP_TEMP_UPDT    4999
#define LOOP_FANCTRL_UPDT 2999
#define LOOP_TACH_UPDT    7919      // prime 1000
#define LOOP_GUI_UPDT     3000
#define LOOP_LED_UPDT     200
#define LOOP_ADC_UPDT     15117

// EEPROM
#define EEPROM_SIG        0x01      // aka version, eeprom will be cleared if this doesn't match
#define EEPROM_SVDELAY    300000

// I2C COMMON
//also need to change in SI2C library because #cpp translation units b/s
//#define SCL_PIN         5
//#define SCL_PORT        PORTD
//#define SDA_PIN         6
//#define SDA_PORT        PORTD
#define SCL_PIN         0
#define SCL_PORT        PORTB
#define SDA_PIN         4
#define SDA_PORT        PORTB
#define I2C_FASTMODE    0
#define I2C_SLOWMODE    0
#define I2C_NOINTERRUPT 1
#define I2C_TIMEOUT     100
#include <SoftI2CMaster.h>

// LCD
#define HASLCD          1
#define LCD_ROTTIME     5000000UL
#define LCD_TIMEOUT     60000000UL
#define LCD_STATUSLEN   12
#define BLANK_3CHAR     "   "
#define BLANK_4CHAR     "    "
// Note that library was modified to not include asm twice
// Otherwise linker would complain loudly...
#include <LiquidCrystal_SI2C.h>

// LED
#define LED_GREEN_PORT    PORTC 
#define LED_GREEN_DDR     DDRC
#define LED_GREEN_PIN     B00100000    
#define LED_RED_PORT      PORTB 
#define LED_RED_DDR       DDRB
#define LED_RED_PIN       B00100000 
#define LED_BLINK_PERIOD  600

// TEMPERATURE SENSOR
#define FAKE_TEMP_OUTPUT  0
#define MLX90614_I2CADDR  0x5A
// RAM
#define MLX90614_RAWIR1   0x04
#define MLX90614_RAWIR2   0x05
#define MLX90614_TA       0x06
#define MLX90614_TOBJ1    0x07
#define MLX90614_TOBJ2    0x08
// EEPROM
#define MLX90614_TOMAX    0x20
#define MLX90614_TOMIN    0x21
#define MLX90614_PWMCTRL  0x22
#define MLX90614_TARANGE  0x23
#define MLX90614_EMISS    0x24
#define MLX90614_CONFIG   0x25
#define MLX90614_ADDR     0x0E
#define MLX90614_ID1      0x3C
#define MLX90614_ID2      0x3D
#define MLX90614_ID3      0x3E
#define MLX90614_ID4      0x3F

// ENCODER
#define ENC_CLKPORT     PORTD
#define ENC_CLKDDR      DDRD
#define ENC_CLKPIN      B00000100       //INT0, D2
#define ENC_DATAPORT    PORTC
#define ENC_DATADDR     DDRC
#define ENC_DATAPIN     B00010000       //A4
#define ENC_DATAPINREG  PINC
#define ENC_BTNPORT     PORTD
#define ENC_BTNDDR      DDRD
#define ENC_BTNPIN      B00001000       //INT1, D3
#define ENC_ENCDB       1000            //Encoder debounce in us
#define ENC_BTNDB       50000           //Button debounce in us

// ADC
#define FAKE_ADC_OUTPUT 1
#define ADC_VPIN        7               //A7
#define ADC_IPIN        6               //A6

// PWM TIMER
#define TIMER1_DEF_FREQ 28000U
#define TIMER1_DDR      DDRB
#define TIMER1_PORT     PORTB
#define TIMER1_A_PIN    B00000010       //D9
#define TIMER1_B_PIN    B00000100       //D10

// FAN CONTROL
#define FANCTRL_SW_DDR    DDRC
#define FANCTRL_SW_PORT   PORTC
#define FANCTRL_SW_PIN    B00000001 
//#define FANCTRL_TEMP_BMRG 5
#define FANCTRL_TEMP_HYST 2
#define FANCTRL_TEMP_OFF  50
#define FANCTRL_TEMP_BOT  70
#define FANCTRL_TEMP_TOP  150
#define FANCTRL_MINCYCLE  8
#define FANCTRL_MAXCYCLE  100
#define FANCTRL_WARNRPM   300
#define FANCTRL_ALARMRPM  200
#define FANCTRL_ACTFANS   B01111111
#define FANCTRL_ACTFANSN  7
#define FANCTRL_FAILTHR   20

#if ((FANCTRL_TEMP_TOP - FANCTRL_TEMP_BOT) > 255)
  #error BAD TEMPERATURE RANGE
#endif

// TACH
#define TACH_SMOOTHNUM  3
#define FANARR_SIZE     8
#define PORTD_SIZE      4
#define PORTD_FIRSTPIN  4
#define PORTD_SHIFT     0
#define PORTC_SIZE      3
#define PORTC_FIRSTPIN  1
#define PORTC_SHIFT     4
#define TACH_F1         4
#define TACH_F2         5
#define TACH_F3         6
#define TACH_F4         7
#define TACH_F5         A1
#define TACH_F6         A2
#define TACH_F7         A3
#define TACH_PORTD_MASK B11110000
#define TACH_PORTC_MASK B00001110
#define TACH_MIN_LENGTH 10000U //10ms minimum time to prevent spurious reads

// ERRORS
#define TEMP_IR_MIN         -39.0
#define TEMP_IR_WARN        200.0
#define TEMP_IR_ALARM       250.0
#define TEMP_SENSOR_MIN     -39.0
#define TEMP_SENSOR_WARN    75.0
#define TEMP_SENSOR_ALARM   85.0
#define TEMP_CHIP_MIN      -39.0
#define TEMP_CHIP_WARN      60.0
#define TEMP_CHIP_ALARM     70.0
#define ADC_V_LOWALARM      4.0
#define ADC_V_HIGHALARM     14.0
#define ADC_I_ALARM         3.0

#define TIMER1_FREQ_ERR           0x11
#define TIMER1_WRONG_PWM          0x12
#define TIMER1_WRONG_PIN          0x13
#define IRSENSOR_EBYTE_ERR        0x21
#define IRSENSOR_NODATA_ERR       0x22
#define IR_TEMPLOW_ERR            0x31
#define IR_TEMPHIGH_ERR           0x32
#define SENSOR_TEMPLOW_ERR        0x3A
#define SENSOR_TEMPHIGH_ERR       0x34
#define CHIP_TEMPLOW_ERR          0x3B
#define CHIP_TEMPHIGH_ERR         0x36
#define FCTRL_RPMTOOLOW           0x41
#define ADC_VTOOLOW               0x5A
#define ADC_VTOOHIGH              0x5B
#define ADC_ITOOHIGH              0x5C
#define FATAL_DOUBLEERREN         1
#define FATAL_DOUBLERR            0x91
// Not used
//#define FATAL_LIST                {TIMER1_FREQ_ERR,IR_TEMPLOW_ERR,IR_TEMPHIGH_ERR}

const char string_0[] PROGMEM = "OK..........";
const char string_1[] PROGMEM = "Timer1Err...";
const char string_2[] PROGMEM = "IRSensorErr.";
const char string_3[] PROGMEM = "TempOutOfBnd";
const char string_4[] PROGMEM = "FanCtrlError";
const char string_5[] PROGMEM = "PSUError....";
const char* const statusStringTable[] PROGMEM = {string_0, string_1, string_2, string_3, string_4, string_5};
//const char* const statusStringTable[] = {string_0, string_1, string_2, string_3, string_4, string_5};
#endif
