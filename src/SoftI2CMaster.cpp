#include "globals.h"
#include "SoftI2CMaster.h"

#if !I2C_HARDWARE
// map the IO register back into the IO address space
#define SDA_DDR       	(_SFR_IO_ADDR(SDA_PORT) - 1)
#define SCL_DDR       	(_SFR_IO_ADDR(SCL_PORT) - 1)
#define SDA_OUT       	_SFR_IO_ADDR(SDA_PORT)
#define SCL_OUT       	_SFR_IO_ADDR(SCL_PORT)
#define SDA_IN		(_SFR_IO_ADDR(SDA_PORT) - 2)
#define SCL_IN		(_SFR_IO_ADDR(SCL_PORT) - 2)

#ifndef __tmp_reg__
#define __tmp_reg__ 0
#endif

// Internal delay functions.
void __attribute__ ((noinline)) i2c_delay_half(void) asm("ass_i2c_delay_half")  __attribute__ ((used));
void __attribute__ ((noinline)) i2c_wait_scl_high(void) asm("ass_i2c_wait_scl_high")  __attribute__ ((used));

void  i2c_delay_half(void)
{ // function call 3 cycles => 3C
#if I2C_DELAY_COUNTER < 1
  __asm__ __volatile__ (" ret");
  // 7 cycles for call and return
#else
  __asm__ __volatile__
    (
     " ldi      r25, %[DELAY]           ;load delay constant   ;; 4C \n\t"
     "_Lidelay: \n\t"
     " dec r25                          ;decrement counter     ;; 4C+xC \n\t"
     " brne _Lidelay                                           ;;5C+(x-1)2C+xC\n\t"
     " ret                                                     ;; 9C+(x-1)2C+xC = 7C+xC"
     : : [DELAY] "M" I2C_DELAY_COUNTER : "r25");
  // 7 cycles + 3 times x cycles
#endif
}

void i2c_wait_scl_high(void)
{
#if I2C_TIMEOUT <= 0
  __asm__ __volatile__
    ("_Li2c_wait_stretch: \n\t"
     " sbis	%[SCLIN],%[SCLPIN]	;wait for SCL high \n\t"
#if __AVR_HAVE_JMP_CALL__
     " jmp	_Li2c_wait_stretch \n\t"
#else
     " rjmp  _Li2c_wait_stretch \n\t"
#endif
     " cln                              ;signal: no timeout \n\t"
     " ret "
     : : [SCLIN] "I" (SCL_IN), [SCLPIN] "I" (SCL_PIN));
#else
  __asm__ __volatile__
    ( " ldi     r27, %[HISTRETCH]       ;load delay counter \n\t"
      " ldi     r26, %[LOSTRETCH] \n\t"
      "_Lwait_stretch: \n\t"
      " clr     __tmp_reg__             ;do next loop 255 times \n\t"
      "_Lwait_stretch_inner_loop: \n\t"
#if __AVR_HAVE_JMP_CALL__
            " call   _Lcheck_scl_level       ;call check function   ;; 12C \n\t"
#else
            " rcall   _Lcheck_scl_level       ;call check function   ;; 12C \n\t"
#endif
      " brpl    _Lstretch_done          ;done if N=0           ;; +1 = 13C\n\t"
      " dec     __tmp_reg__             ;dec inner loop counter;; +1 = 14C\n\t"
      " brne    _Lwait_stretch_inner_loop                      ;; +2 = 16C\n\t"
      " sbiw    r26,1                   ;dec outer loop counter \n\t"
      " brne    _Lwait_stretch          ;continue with outer loop \n\t"
      " sen                             ;timeout -> set N-bit=1 \n\t"
#if __AVR_HAVE_JMP_CALL__
      " jmp _Lwait_return              ;and return with N=1\n\t"
#else
      " rjmp _Lwait_return              ;and return with N=1\n\t"
#endif
      "_Lstretch_done:                  ;SCL=1 sensed \n\t"
      " cln                             ;OK -> clear N-bit \n\t"
#if __AVR_HAVE_JMP_CALL__
      " jmp _Lwait_return              ; and return with N=0 \n\t"
#else
      " rjmp _Lwait_return              ; and return with N=0 \n\t"
#endif
      "_Lcheck_scl_level:                                      ;; call = 3C\n\t"
      " cln                                                    ;; +1C = 4C \n\t"
      " sbic	%[SCLIN],%[SCLPIN]      ;skip if SCL still low ;; +2C = 6C \n\t"
#if __AVR_HAVE_JMP_CALL__
            " jmp    _Lscl_high                                     ;; +0C = 6C \n\t"
#else
            " rjmp    _Lscl_high                                     ;; +0C = 6C \n\t"
#endif
      " sen                                                    ;; +1 = 7C\n\t "
      "_Lscl_high: "
      " nop                                                    ;; +1C = 8C \n\t"
      " ret                             ;return N-Bit=1 if low ;; +4 = 12C\n\t"

      "_Lwait_return:"
      : : [SCLIN] "I" (SCL_IN), [SCLPIN] "I" (SCL_PIN),
	[HISTRETCH] "M" (I2C_MAX_STRETCH>>8),
	[LOSTRETCH] "M" (I2C_MAX_STRETCH&0xFF)
      : "r26", "r27");
#endif
}
#endif // !I2C_HARDWARE

bool i2c_init(void)
#if I2C_HARDWARE
{
#if I2C_PULLUP
  digitalWrite(SDA, 1);
  digitalWrite(SCL, 1);
#else
  digitalWrite(SDA, 0);
  digitalWrite(SCL, 0);
#endif
#if ((I2C_CPUFREQ/SCL_CLOCK)-16)/2 < 250
  TWSR = 0;                         /* no prescaler */
  TWBR = ((I2C_CPUFREQ/SCL_CLOCK)-16)/2;  /* must be > 10 for stable operation */
#else
  TWSR = (1<<TWPS0); // prescaler is 4
  TWBR = ((I2C_CPUFREQ/SCL_CLOCK)-16)/8;
#endif
  return (digitalRead(SDA) != 0 && digitalRead(SCL) != 0);
}
#else
{
  __asm__ __volatile__
    (" cbi      %[SDADDR],%[SDAPIN]     ;release SDA \n\t"
     " cbi      %[SCLDDR],%[SCLPIN]     ;release SCL \n\t"
#if I2C_PULLUP
     " sbi      %[SDAOUT],%[SDAPIN]     ;enable SDA pull-up\n\t"
#else
     " cbi      %[SDAOUT],%[SDAPIN]     ;clear SDA output value \n\t"
#endif
#if I2C_PULLUP
     " sbi      %[SCLOUT],%[SCLPIN]     ;enable SCL pull-up\n\t"
#else
     " cbi      %[SCLOUT],%[SCLPIN]     ;clear SCL output value \n\t"
#endif
     " clr      r24                     ;set return value to false \n\t"
     " clr      r25                     ;set return value to false \n\t"
     " sbis     %[SDAIN],%[SDAPIN]      ;check for SDA high\n\t"
     " ret                              ;if low return with false \n\t"
     " sbis     %[SCLIN],%[SCLPIN]      ;check for SCL high \n\t"
     " ret                              ;if low return with false \n\t"
     " ldi      r24,1                   ;set return value to true \n\t"
     " ret "
     : :
       [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN),
       [SCLIN] "I" (SCL_IN), [SCLOUT] "I" (SCL_OUT),
       [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN),
       [SDAIN] "I" (SDA_IN), [SDAOUT] "I" (SDA_OUT));
  return true;
}
#endif

bool  i2c_start(uint8_t addr)
#if I2C_HARDWARE
{
  uint8_t   twst;
#if I2C_TIMEOUT
  uint32_t start = millis();
#endif

  // send START condition
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

  // wait until transmission completed
  while(!(TWCR & (1<<TWINT))) {
#if I2C_TIMEOUT
    if (millis() - start > I2C_TIMEOUT) return false;
#endif
  }

  // check value of TWI Status Register. Mask prescaler bits.
  twst = TW_STATUS & 0xF8;
  if ( (twst != TW_START) && (twst != TW_REP_START)) return false;

  // send device address
  TWDR = addr;
  TWCR = (1<<TWINT) | (1<<TWEN);

  // wail until transmission completed and ACK/NACK has been received
  while(!(TWCR & (1<<TWINT))) {
#if I2C_TIMEOUT
    if (millis() - start > I2C_TIMEOUT) return false;
#endif
  }

  // check value of TWI Status Register. Mask prescaler bits.
  twst = TW_STATUS & 0xF8;
  if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return false;

  return true;
}
#else
{
  __asm__ __volatile__
    (
#if I2C_NOINTERRUPT
     " cli                              ;clear IRQ bit \n\t"
#endif
     " sbis     %[SCLIN],%[SCLPIN]      ;check for clock stretching slave\n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#else
            " rcall    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#endif
#if I2C_PULLUP
     " cbi      %[SDAOUT],%[SDAPIN]     ;disable pull-up \n\t"
#endif
     " sbi      %[SDADDR],%[SDAPIN]     ;force SDA low  \n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_delay_half      ;wait T/2 \n\t"
            " call    ass_i2c_write           ;now write address \n\t"
#else
            " rcall    ass_i2c_delay_half      ;wait T/2 \n\t"
            " rcall    ass_i2c_write           ;now write address \n\t"
#endif
     " ret"
     : : [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN),
       [SDAOUT] "I" (SDA_OUT), [SCLOUT] "I" (SCL_OUT),
       [SCLIN] "I" (SCL_IN),[SCLPIN] "I" (SCL_PIN));
  return true; // we never return here!
}
#endif

bool  i2c_rep_start(uint8_t addr)
#if I2C_HARDWARE
{
  return i2c_start(addr);
}
#else
{
  __asm__ __volatile__

    (
#if I2C_NOINTERRUPT
     " cli \n\t"
#endif
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low \n\t"
#if __AVR_HAVE_JMP_CALL__
            " call     ass_i2c_delay_half  ;delay  T/2 \n\t"
#else
            " rcall     ass_i2c_delay_half  ;delay  T/2 \n\t"
#endif
     " cbi	%[SDADDR],%[SDAPIN]	;release SDA \n\t"
#if I2C_PULLUP
     " sbi      %[SDAOUT],%[SDAPIN]     ;enable SDA pull-up \n\t"
#endif
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2 \n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2 \n\t"
#endif
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL \n\t"
#if I2C_PULLUP
     " sbi      %[SCLOUT],%[SCLPIN]	;enable SCL pull-up \n\t"
#endif
#if __AVR_HAVE_JMP_CALL__
            " call     ass_i2c_delay_half  ;delay  T/2 \n\t"
#else
            " rcall     ass_i2c_delay_half  ;delay  T/2 \n\t"
#endif
     " sbis     %[SCLIN],%[SCLPIN]      ;check for clock stretching slave\n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#else
            " rcall    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#endif
#if I2C_PULLUP
     " cbi 	%[SDAOUT],%[SDAPIN]	;disable SDA pull-up\n\t"
#endif
     " sbi 	%[SDADDR],%[SDAPIN]	;force SDA low \n\t"
#if __AVR_HAVE_JMP_CALL__
            " call     ass_i2c_delay_half  ;delay  T/2 \n\t"
            " call    ass_i2c_write       \n\t"
#else
            " rcall     ass_i2c_delay_half  ;delay  T/2 \n\t"
            " rcall    ass_i2c_write       \n\t"
#endif
     " ret"
     : : [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN),
       [SCLIN] "I" (SCL_IN), [SCLOUT] "I" (SCL_OUT), [SDAOUT] "I" (SDA_OUT),
       [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN));
  return true; // just to fool the compiler
}
#endif

bool  i2c_start_wait(uint8_t addr)
#if I2C_HARDWARE
{
  uint8_t   twst;
  uint16_t maxwait = I2C_MAXWAIT;
#if I2C_TIMEOUT
  uint32_t start = millis();
#endif

  while (true) {
    // send START condition
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

    // wait until transmission completed
    while(!(TWCR & (1<<TWINT))) {
#if I2C_TIMEOUT
    if (millis() - start > I2C_TIMEOUT) return false;
#endif
    }

    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
    if ( (twst != TW_START) && (twst != TW_REP_START)) continue;

    // send device address
    TWDR = addr;
    TWCR = (1<<TWINT) | (1<<TWEN);

    // wail until transmission completed
    while(!(TWCR & (1<<TWINT))) {
#if I2C_TIMEOUT
      if (millis() - start > I2C_TIMEOUT) return false;
#endif
    }

    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
    if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) )
      {
	/* device busy, send stop condition to terminate write operation */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO)) {
#if I2C_TIMEOUT
	  if (millis() - start > I2C_TIMEOUT) return false;
#endif
	}

	if (maxwait)
	  if (--maxwait == 0)
	    return false;

	continue;
      }
    //if( twst != TW_MT_SLA_ACK) return 1;
    return true;
  }
}
#else
{
 __asm__ __volatile__
   (
    " push	r24                     ;save original parameter \n\t"
#if I2C_MAXWAIT
    " ldi     r31, %[HIMAXWAIT]         ;load max wait counter \n\t"
    " ldi     r30, %[LOMAXWAIT]         ;load low byte \n\t"
#endif
    "_Li2c_start_wait1: \n\t"
    " pop       r24                     ;restore original parameter\n\t"
    " push      r24                     ;and save again \n\t"
#if I2C_NOINTERRUPT
    " cli                               ;disable interrupts \n\t"
#endif
    " sbis     %[SCLIN],%[SCLPIN]      ;check for clock stretching slave\n\t"
#if __AVR_HAVE_JMP_CALL__
           " call    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#else
           " rcall    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#endif
#if I2C_PULLUP
     " cbi      %[SDAOUT],%[SDAPIN]     ;disable pull-up \n\t"
#endif
    " sbi 	%[SDADDR],%[SDAPIN]	;force SDA low \n\t"
#if __AVR_HAVE_JMP_CALL__
           " call  ass_i2c_delay_half  ;delay T/2 \n\t"
           " call  ass_i2c_write           ;write address \n\t"
#else
           " rcall  ass_i2c_delay_half  ;delay T/2 \n\t"
           " rcall  ass_i2c_write           ;write address \n\t"
#endif
    " tst	r24		        ;if device not busy -> done \n\t"
    " brne	_Li2c_start_wait_done \n\t"
#if __AVR_HAVE_JMP_CALL__
           " call  ass_i2c_stop            ;terminate write & enable IRQ \n\t"
#else
           " rcall  ass_i2c_stop            ;terminate write & enable IRQ \n\t"
#endif
#if I2C_MAXWAIT
    " sbiw      r30,1                   ;decrement max wait counter\n\t"
    " breq       _Li2c_start_wait_done  ;if zero reached, exit with false -> r24 already zero!\n\t"
#endif
#if __AVR_HAVE_JMP_CALL__
           " jmp   _Li2c_start_wait1   ;device busy, poll ack again \n\t"
#else
           " rjmp   _Li2c_start_wait1   ;device busy, poll ack again \n\t"
#endif
    "_Li2c_start_wait_done: \n\t"
    " clr       r25                     ;clear high byte of return value\n\t"
    " pop       __tmp_reg__             ;pop off orig argument \n\t"
    " ret "
    : : [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN), [SDAOUT] "I" (SDA_OUT),
      [SCLIN] "I" (SCL_IN), [SCLPIN] "I" (SCL_PIN),
      [HIMAXWAIT] "M" (I2C_MAXWAIT>>8),
      [LOMAXWAIT] "M" (I2C_MAXWAIT&0xFF)
    : "r30", "r31" );
 return true; // fooling the compiler
}
#endif

void  i2c_stop(void)
#if I2C_HARDWARE
{
#if I2C_TIMEOUT
  uint32_t start = millis();
#endif
  /* send stop condition */
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);

  // wait until stop condition is executed and bus released
  while(TWCR & (1<<TWSTO)) {
#if I2C_TIMEOUT
    if (millis() - start > I2C_TIMEOUT) return;
#endif
  }
}
#else
{
  __asm__ __volatile__
    (
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     " sbi      %[SCLDDR],%[SCLPIN]     ;force SCL low \n\t"
#if I2C_PULLUP
     " cbi      %[SDAOUT],%[SDAPIN]     ;disable pull-up \n\t"
#endif
     " sbi      %[SDADDR],%[SDAPIN]     ;force SDA low \n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_delay_half      ;T/2 delay \n\t"
#else
            " rcall    ass_i2c_delay_half      ;T/2 delay \n\t"
#endif
     " cbi      %[SCLDDR],%[SCLPIN]     ;release SCL \n\t"
#if I2C_PULLUP
     " sbi      %[SCLOUT],%[SCLPIN]	;enable SCL pull-up \n\t"
#endif
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_delay_half      ;T/2 delay \n\t"
#else
            " rcall    ass_i2c_delay_half      ;T/2 delay \n\t"
#endif
     " sbis     %[SCLIN],%[SCLPIN]      ;check for clock stretching slave\n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#else
            " rcall    ass_i2c_wait_scl_high   ;wait until SCL=H\n\t"
#endif
     " cbi      %[SDADDR],%[SDAPIN]     ;release SDA \n\t"
#if I2C_PULLUP
     " sbi      %[SDAOUT],%[SDAPIN]     ;enable SDA pull-up \n\t"
#endif
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_delay_half \n\t"
#else
            " rcall    ass_i2c_delay_half \n\t"
#endif
#if I2C_NOINTERRUPT
     " sei                              ;enable interrupts again!\n\t"
#endif
     : : [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN), [SCLIN] "I" (SCL_IN),
       [SDAOUT] "I" (SDA_OUT), [SCLOUT] "I" (SCL_OUT),
       [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN));
}
#endif


bool i2c_write(uint8_t value)
#if I2C_HARDWARE
{
  uint8_t   twst;
#if I2C_TIMEOUT
  uint32_t start = millis();
#endif


  // send data to the previously addressed device
  TWDR = value;
  TWCR = (1<<TWINT) | (1<<TWEN);

  // wait until transmission completed
  while(!(TWCR & (1<<TWINT))) {
#if I2C_TIMEOUT
    if (millis() - start > I2C_TIMEOUT) return false;
#endif
  }

  // check value of TWI Status Register. Mask prescaler bits
  twst = TW_STATUS & 0xF8;
  if( twst != TW_MT_DATA_ACK) return false;
  return true;
}
#else
{
  __asm__ __volatile__
    (
     " sec                              ;set carry flag \n\t"
     " rol      r24                     ;shift in carry and shift out MSB \n\t"
#if __AVR_HAVE_JMP_CALL__
            " jmp _Li2c_write_first \n\t"
#else
            " rjmp _Li2c_write_first \n\t"
#endif
     "_Li2c_write_bit:\n\t"
     " lsl      r24                     ;left shift into carry ;; 1C\n\t"
     "_Li2c_write_first:\n\t"
     " breq     _Li2c_get_ack           ;jump if TXreg is empty;; +1 = 2C \n\t"
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     " sbi      %[SCLDDR],%[SCLPIN]     ;force SCL low         ;; +2 = 4C \n\t"
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
     " brcc     _Li2c_write_low                                ;;+1/+2=5/6C\n\t"
     " nop                                                     ;; +1 = 7C \n\t"
     " cbi %[SDADDR],%[SDAPIN]	        ;release SDA           ;; +2 = 9C \n\t"
#if I2C_PULLUP
     " sbi      %[SDAOUT],%[SDAPIN]     ;enable SDA pull-up \n\t"
#endif
#if __AVR_HAVE_JMP_CALL__
            " jmp      _Li2c_write_high                              ;; +2 = 11C \n\t"
#else
            " rjmp      _Li2c_write_high                              ;; +2 = 11C \n\t"
#endif
     "_Li2c_write_low: \n\t"
#if I2C_PULLUP
     " cbi      %[SDAOUT],%[SDAPIN]     ;disable pull-up \n\t"
#endif
     " sbi	%[SDADDR],%[SDAPIN]	;force SDA low         ;; +2 = 9C \n\t"
#if __AVR_HAVE_JMP_CALL__
            " jmp  _Li2c_write_high                               ;;+2 = 11C \n\t"
#else
            " rjmp  _Li2c_write_high                               ;;+2 = 11C \n\t"
#endif
     "_Li2c_write_high: \n\t"
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call     ass_i2c_delay_half  ;delay T/2             ;;+X = 11C+X\n\t"
#else
            " rcall     ass_i2c_delay_half  ;delay T/2             ;;+X = 11C+X\n\t"
#endif
#endif
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL           ;;+2 = 13C+X\n\t"
#if I2C_PULLUP
     " sbi      %[SCLOUT],%[SCLPIN]	;enable SCL pull-up \n\t"
#endif
     " cln                              ;clear N-bit           ;;+1 = 14C+X\n\t"
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
     " sbis	%[SCLIN],%[SCLPIN]	;check for SCL high    ;;+2 = 16C+X\n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_wait_scl_high \n\t"
#else
            " rcall    ass_i2c_wait_scl_high \n\t"
#endif
     " brpl     _Ldelay_scl_high                              ;;+2 = 18C+X\n\t"
     "_Li2c_write_return_false: \n\t"
     " clr      r24                     ; return false because of timeout \n\t"
#if __AVR_HAVE_JMP_CALL__
            " jmp     _Li2c_write_return \n\t"
#else
            " rjmp     _Li2c_write_return \n\t"
#endif
     "_Ldelay_scl_high: \n\t"
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2             ;;+X= 18C+2X\n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2             ;;+X= 18C+2X\n\t"
#endif
#endif
#if __AVR_HAVE_JMP_CALL__
            " jmp  _Li2c_write_bit \n\t"
#else
            " rjmp  _Li2c_write_bit \n\t"
#endif
     "              ;; +2 = 20C +2X for one bit-loop \n\t"
     "_Li2c_get_ack: \n\t"
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low ;; +2 = 5C \n\t"
     " nop \n\t"
     " nop \n\t"
     " cbi	%[SDADDR],%[SDAPIN]	;release SDA ;;+2 = 7C \n\t"
#if I2C_PULLUP
     " sbi      %[SDAOUT],%[SDAPIN]     ;enable SDA pull-up \n\t"
#endif
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2 ;; +X = 7C+X \n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2 ;; +X = 7C+X \n\t"
#endif
#endif
     " clr	r25                                            ;; 17C+2X \n\t"
     " clr	r24		        ;return 0              ;; 14C + X \n\t"
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL ;; +2 = 9C+X\n\t"
#if I2C_PULLUP
     " sbi      %[SCLOUT],%[SCLPIN]	;enable SCL pull-up \n\t"
#endif
     "_Li2c_ack_wait: \n\t"
     " cln                              ; clear N-bit          ;; 10C + X\n\t"
     " nop \n\t"
     " sbis	%[SCLIN],%[SCLPIN]	;wait SCL high         ;; 12C + X \n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_wait_scl_high \n\t"
#else
            " rcall    ass_i2c_wait_scl_high \n\t"
#endif
     " brmi     _Li2c_write_return_false                       ;; 13C + X \n\t "
     " sbis	%[SDAIN],%[SDAPIN]      ;if SDA hi -> return 0 ;; 15C + X \n\t"
     " ldi	r24,1                   ;return true           ;; 16C + X \n\t"
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2             ;; 16C + 2X \n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2             ;; 16C + 2X \n\t"
#endif
#endif
     "_Li2c_write_return: \n\t"
     " nop \n\t "
     " nop \n\t "
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low so SCL=H is short\n\t"
     " ret \n\t"
     "              ;; + 4 = 17C + 2X for acknowldge bit"
     ::
      [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN), [SCLIN] "I" (SCL_IN),
      [SDAOUT] "I" (SDA_OUT), [SCLOUT] "I" (SCL_OUT),
      [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN), [SDAIN] "I" (SDA_IN));
  return true; // fooling the compiler
}
#endif

uint8_t i2c_read(bool last)
#if I2C_HARDWARE
{
#if I2C_TIMEOUT
  uint32_t start = millis();
#endif

  TWCR = (1<<TWINT) | (1<<TWEN) | (last ? 0 : (1<<TWEA));
  while(!(TWCR & (1<<TWINT))) {
#if I2C_TIMEOUT
    if (millis() - start > I2C_TIMEOUT) return 0xFF;
#endif
  }
  return TWDR;
}
#else
{
  __asm__ __volatile__
    (
     " ldi	r23,0x01 \n\t"
     "_Li2c_read_bit: \n\t"
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low         ;; 2C \n\t"
     " cbi	%[SDADDR],%[SDAPIN]	;release SDA(prev. ACK);; 4C \n\t"
#if I2C_PULLUP
     " sbi      %[SDAOUT],%[SDAPIN]     ;enable SDA pull-up \n\t"
#endif
     " nop \n\t"
     " nop \n\t"
     " nop \n\t"
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2             ;; 4C+X \n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2             ;; 4C+X \n\t"
#endif
#endif
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL           ;; 6C + X \n\t"
#if I2C_PULLUP
     " sbi      %[SCLOUT],%[SCLPIN]	;enable SCL pull-up \n\t"
#endif
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2             ;; 6C + 2X \n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2             ;; 6C + 2X \n\t"
#endif
#endif
     " cln                              ; clear N-bit          ;; 7C + 2X \n\t"
     " nop \n\t "
     " nop \n\t "
     " nop \n\t "
     " sbis     %[SCLIN], %[SCLPIN]     ;check for SCL high    ;; 9C +2X \n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_wait_scl_high \n\t"
#else
            " rcall    ass_i2c_wait_scl_high \n\t"
#endif
     " brmi     _Li2c_read_return       ;return if timeout     ;; 10C + 2X\n\t"
     " clc		  	        ;clear carry flag      ;; 11C + 2X\n\t"
     " sbic	%[SDAIN],%[SDAPIN]	;if SDA is high        ;; 11C + 2X\n\t"
     " sec			        ;set carry flag        ;; 12C + 2X\n\t"
     " rol	r23		        ;store bit             ;; 13C + 2X\n\t"
     " brcc	_Li2c_read_bit	        ;while receiv reg not full \n\t"
     "                         ;; 15C + 2X for one bit loop \n\t"

     "_Li2c_put_ack: \n\t"
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     " sbi	%[SCLDDR],%[SCLPIN]	;force SCL low         ;; 2C \n\t"
     " cpi	r24,0                                          ;; 3C \n\t"
     " breq	_Li2c_put_ack_low	;if (ack=0) ;; 5C \n\t"
     " cbi	%[SDADDR],%[SDAPIN]	;release SDA \n\t"
#if I2C_PULLUP
     " sbi      %[SDAOUT],%[SDAPIN]     ;enable SDA pull-up \n\t"
#endif
#if __AVR_HAVE_JMP_CALL__
            " jmp   _Li2c_put_ack_high \n\t"
#else
            " rjmp   _Li2c_put_ack_high \n\t"
#endif
     "_Li2c_put_ack_low:                ;else \n\t"
#if I2C_PULLUP
     " cbi      %[SDAOUT],%[SDAPIN]     ;disable pull-up \n\t"
#endif
     " sbi	%[SDADDR],%[SDAPIN]	;force SDA low         ;; 7C \n\t"
     "_Li2c_put_ack_high: \n\t"
     " nop \n\t "
     " nop \n\t "
     " nop \n\t "
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2             ;; 7C + X \n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2             ;; 7C + X \n\t"
#endif
#endif
     " cbi	%[SCLDDR],%[SCLPIN]	;release SCL           ;; 9C +X \n\t"
#if I2C_PULLUP
     " sbi      %[SCLOUT],%[SCLPIN]	;enable SCL pull-up \n\t"
#endif
     " cln                              ;clear N               ;; +1 = 10C\n\t"
     " nop \n\t "
     " nop \n\t "
     " sbis	%[SCLIN],%[SCLPIN]	;wait SCL high         ;; 12C + X\n\t"
#if __AVR_HAVE_JMP_CALL__
            " call    ass_i2c_wait_scl_high \n\t"
#else
            " rcall    ass_i2c_wait_scl_high \n\t"
#endif
#if I2C_DELAY_COUNTER >= 1
#if __AVR_HAVE_JMP_CALL__
            " call ass_i2c_delay_half  ;delay T/2             ;; 11C + 2X\n\t"
#else
            " rcall ass_i2c_delay_half  ;delay T/2             ;; 11C + 2X\n\t"
#endif
#endif
     "_Li2c_read_return: \n\t"
     " nop \n\t "
     " nop \n\t "
#if I2C_PULLUP
     " cbi      %[SCLOUT],%[SCLPIN]     ;disable SCL pull-up \n\t"
#endif
     "sbi	%[SCLDDR],%[SCLPIN]	;force SCL low so SCL=H is short\n\t"
     " mov	r24,r23                                        ;; 12C + 2X \n\t"
     " clr	r25                                            ;; 13 C + 2X\n\t"
     " ret                                                     ;; 17C + X"
     ::
      [SCLDDR] "I"  (SCL_DDR), [SCLPIN] "I" (SCL_PIN), [SCLIN] "I" (SCL_IN),
      [SDAOUT] "I" (SDA_OUT), [SCLOUT] "I" (SCL_OUT),
      [SDADDR] "I"  (SDA_DDR), [SDAPIN] "I" (SDA_PIN), [SDAIN] "I" (SDA_IN)
     );
  return ' '; // fool the compiler!
}
#endif

#pragma GCC diagnostic pop


