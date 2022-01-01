/* Arduino SoftI2C library.
 *
 * Version 2.1.6
 *
 * Copyright (C) 2013-2021, Bernhard Nebel and Peter Fleury
 *
 * This is a very fast and very light-weight software I2C-master library
 * written in assembler. It is based on Peter Fleury's I2C software
 * library: http://homepage.hispeed.ch/peterfleury/avr-software.html
 * Recently, the hardware implementation has been added to the code,
 * which can be enabled by defining I2C_HARDWARE.
 *
 * This file is part of SoftI2CMaster https://github.com/felias-fogg/SoftI2CMaster.
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino I2cMaster Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/* In order to use the library, you need to define SDA_PIN, SCL_PIN,
 * SDA_PORT and SCL_PORT before including this file.  Have a look at
 * http://www.arduino.cc/en/Reference/PortManipulation for finding out
 * which values to use. For example, if you use digital pin 3 (corresponding
 * to PD3) for SDA and digital pin 13 (corresponding to PB5)
 * for SCL on a standard Arduino,
 * you have to use the following definitions:
 * #define SDA_PIN 3
 * #define SDA_PORT PORTD
 * #define SCL_PIN 5
 * #define SCL_PORT PORTB
 *
 * Alternatively, you can define the compile time constant I2C_HARDWARE,
 * in which case the TWI hardware is used. In this case you have to use
 * the standard SDA/SCL pins (and, of course, the chip needs to support
 * this).
 *
 * You can also define the following constants (see also below):
 ' - I2C_PULLUP = 1 meaning that internal pullups should be used
 * - I2C_CPUFREQ, when changing CPU clock frequency dynamically
 * - I2C_FASTMODE = 1 meaning that the I2C bus allows speeds up to 400 kHz
 * - I2C_SLOWMODE = 1 meaning that the I2C bus will allow only up to 25 kHz
 * - I2C_NOINTERRUPT = 1 in order to prohibit interrupts while
 *   communicating (see below). This can be useful if you use the library
 *   for communicating with SMbus devices, which have timeouts.
 *   Note, however, that interrupts are disabled from issuing a start condition
 *   until issuing a stop condition. So use this option with care!
 * - I2C_TIMEOUT = 0..10000 msec in order to return from the I2C functions
 *   in case of a I2C bus lockup (i.e., SCL constantly low). 0 means no timeout.
 * - I2C_MAXWAIT = 0..32767 number of retries in i2c_start_wait. 0 means never stop.
 */

/* Changelog:
 * Version 2.1.7
 * - ArminJo: replaced all calls and jmps by rcalls and rjmps for CPUs not having call and jmp
 * Version 2.1.6
 * - adapted SlowSoftWire to make it comparable to SlowSoftI2C (and a few other minor things)
 * Version 2.1.5
 * - replaced all rcalls and rjmps by calls and jmps
 * Version 2.1.4
 * - fixed bug in Softwire conerning repeated starts
 * Version 2.1.3
 * - removed WireEmu and fixed bug in Eeprom24AA1025SoftI2C.ino
 * Version 2.1
 * - added conditional to check whether it is the right MCU type
 * Version 2.0
 * - added hardware support as well.
 * Version 1.4:
 * - added "maximum retry" in i2c_start_wait in order to avoid lockup
 * - added "internal pullups", but be careful since the option stretches the I2C specs
 * Version 1.3:
 * - added "__attribute__ ((used))" for all functions declared with "__attribute__ ((noinline))"
 *   Now the module is also usable in Arduino 1.6.11+
 * Version 1.2:
 * - added pragma to avoid "unused parameter warnings" (suggestion by Walter)
 * - replaced wrong license file
 * Version 1.1:
 * - removed I2C_CLOCK_STRETCHING
 * - added I2C_TIMEOUT time in msec (0..10000) until timeout or 0 if no timeout
 * - changed i2c_init to return true if and only if both SDA and SCL are high
 * - changed interrupt disabling so that the previous IRQ state is restored
 * Version 1.0: basic functionality
 */

#ifndef __AVR_ARCH__
#error "Not an AVR MCU! Use 'SlowSoftI2CMaster' library instead of 'SoftI2CMaster'!"
#else

#ifndef _SOFTI2C_H
#define _SOFTI2C_H   1

#include <avr/io.h>
#include <Arduino.h>
#include <util/twi.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wunused-parameter"



// Init function. Needs to be called once in the beginning.
// Returns false if SDA or SCL are low, which probably means
// a I2C bus lockup or that the lines are not pulled up.
bool __attribute__ ((noinline)) i2c_init(void) __attribute__ ((used));

// Start transfer function: <addr> is the 8-bit I2C address (including the R/W
// bit).
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_start(uint8_t addr) __attribute__ ((used));

// Similar to start function, but wait for an ACK! Will timeout if I2C_MAXWAIT > 0.
bool  __attribute__ ((noinline)) i2c_start_wait(uint8_t addr) __attribute__ ((used));

// Repeated start function: After having claimed the bus with a start condition,
// you can address another or the same chip again without an intervening
// stop condition.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_rep_start(uint8_t addr) __attribute__ ((used));

// Issue a stop condition, freeing the bus.
void __attribute__ ((noinline)) i2c_stop(void) asm("ass_i2c_stop") __attribute__ ((used));

// Write one byte to the slave chip that had been addressed
// by the previous start call. <value> is the byte to be sent.
// Return: true if the slave replies with an "acknowledge", false otherwise
bool __attribute__ ((noinline)) i2c_write(uint8_t value) asm("ass_i2c_write") __attribute__ ((used));


// Read one byte. If <last> is true, we send a NAK after having received
// the byte in order to terminate the read sequence.
uint8_t __attribute__ ((noinline)) i2c_read(bool last) __attribute__ ((used));

// If you want to use the TWI hardware, you have to define I2C_HARDWARE to be 1
#ifndef I2C_HARDWARE
#define I2C_HARDWARE 0
#endif

#if I2C_HARDWARE
#ifndef TWDR
#error This chip does not support hardware I2C. Please undefine I2C_HARDWARE
#endif
#endif

// You can set I2C_CPUFREQ independently of F_CPU if you
// change the CPU frequency on the fly. If you do not define it,
// it will use the value of F_CPU
#ifndef I2C_CPUFREQ
#define I2C_CPUFREQ F_CPU
#endif

// If I2C_FASTMODE is set to 1, then the highest possible frequency below 400kHz
// is selected. Be aware that not all slave chips may be able to deal with that!
#ifndef I2C_FASTMODE
#define I2C_FASTMODE 0
#endif

// If I2C_FASTMODE is not defined or defined to be 0, then you can set
// I2C_SLOWMODE to 1. In this case, the I2C frequency will not be higher
// than 25KHz. This could be useful for problematic buses with high pull-ups
// and high capacitance.
#ifndef I2C_SLOWMODE
#define I2C_SLOWMODE 0
#endif

// If I2C_PULLUP is set to 1, then the internal pull-up resistors are used.
// This does not conform with the I2C specs, since the bus lines will be
// temporarily in high-state and the internal resistors have roughly 50k.
// With low bus speeds und short buses it usually works, though (hopefully).
#ifndef I2C_PULLUP
#define I2C_PULLUP 0
#endif

// if I2C_NOINTERRUPT is 1, then the I2C routines are not interruptible.
// This is most probably only necessary if you are using a 1MHz system clock,
// you are communicating with a SMBus device, and you want to avoid timeouts.
// Be aware that the interrupt bit is enabled after each call. So the
// I2C functions should not be called in interrupt routines or critical regions.
#ifndef I2C_NOINTERRUPT
#define I2C_NOINTERRUPT 0
#endif

// I2C_TIMEOUT can be set to a value between 1 and 10000.
// If it is defined and nonzero, it leads to a timeout if the
// SCL is low longer than I2C_TIMEOUT milliseconds, i.e., max timeout is 10 sec
#ifndef I2C_TIMEOUT
#define I2C_TIMEOUT 0
#else
#if I2C_TIMEOUT > 10000
#error I2C_TIMEOUT is too large
#endif
#endif

// I2C_MAXWAIT can be set to any value between 0 and 32767. 0 means no time out.
#ifndef I2C_MAXWAIT
#define I2C_MAXWAIT 500
#else
#if I2C_MAXWAIT > 32767 || I2C_MAXWAIT < 0
#error Illegal I2C_MAXWAIT value
#endif
#endif

#define I2C_TIMEOUT_DELAY_LOOPS (I2C_CPUFREQ/1000UL)*I2C_TIMEOUT/4000UL
#if I2C_TIMEOUT_DELAY_LOOPS < 1
#define I2C_MAX_STRETCH 1
#else
#if I2C_TIMEOUT_DELAY_LOOPS > 60000UL
#define I2C_MAX_STRETCH 60000UL
#else
#define I2C_MAX_STRETCH I2C_TIMEOUT_DELAY_LOOPS
#endif
#endif

#if I2C_FASTMODE
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ/350000L)/2-18)/3)
#define SCL_CLOCK 400000UL
#else
#if I2C_SLOWMODE
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ/23500L)/2-18)/3)
#define SCL_CLOCK 25000UL
#else
#define I2C_DELAY_COUNTER (((I2C_CPUFREQ/90000L)/2-18)/3)
#define SCL_CLOCK 100000UL
#endif
#endif

// constants for reading & writing
#define I2C_READ    1
#define I2C_WRITE   0

#endif
#endif