#ifndef _SoftWire_h
#define _SoftWire_h

#include <inttypes.h>
#include "Stream.h"

#ifndef I2C_BUFFER_LENGTH
  #define I2C_BUFFER_LENGTH 32
#endif

// WIRE_HAS_END means Wire has end()
#define WIRE_HAS_END 1

class SoftWire : public Stream
{
private:
  uint8_t rxBuffer[I2C_BUFFER_LENGTH];
  uint8_t rxBufferIndex;
  uint8_t rxBufferLength;
  uint8_t transmitting;
  uint8_t error;
public:
  SoftWire(void);
  void begin(void);
  void end(void);
  void setClock(uint32_t _);
  void beginTransmission(uint8_t address);
  void beginTransmission(int address);
  uint8_t endTransmission(uint8_t sendStop);
  uint8_t endTransmission(void);
  size_t write(uint8_t data);
  size_t write(const uint8_t *data, size_t quantity);
  uint8_t requestFrom(uint8_t address, uint8_t quantity, uint32_t iaddress, uint8_t isize, uint8_t sendStop);
  uint8_t requestFrom(uint8_t address, uint8_t quantity, uint8_t sendStop);
  uint8_t requestFrom(int address, int quantity, int sendStop);
  uint8_t requestFrom(uint8_t address, uint8_t quantity);
  uint8_t requestFrom(int address, int quantity);
  int available(void);
  int read(void);
  int peek(void);
  void flush(void);
  // inline size_t write(unsigned long n);
  // inline size_t write(long n);
  // inline size_t write(unsigned int n);
  // inline size_t write(int n);
  inline size_t write(unsigned long n) { return write((uint8_t)n); }
  inline size_t write(long n) { return write((uint8_t)n); }
  inline size_t write(unsigned int n) { return write((uint8_t)n); }
  inline size_t write(int n) { return write((uint8_t)n); }
  using Print::write;
};

extern SoftWire Wire;

#endif