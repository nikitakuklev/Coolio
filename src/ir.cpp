#include "ir.h"
#include "errors.h"
#include "extras.h"
#include "SoftI2CMaster.h"

// Credits to AdaFruit and SoftI2C library for most of this code

void setupIRSensor() {
  #if (DEBUG>1)
    Serial.println("Setting up IR sensor");
  #endif
  // Skipping init function - it just checks if SDA/SCL are high by setting them as inputs
  //i2c_init();
}

void updateTemps() {
  #if FAKE_TEMP_OUTPUT
    static int8_t sign = 1;
    #if (DEBUG>1)
      //Serial.println(" Updating all temps (fake)");
    #endif
    //getIRTemp();
    //getSensorTemp();
    //temp_chip = getChipTemperature();
    // fake output over whole range
    temp_IR += (5*sign);
    if (temp_IR > TEMP_IR_ALARM || temp_IR < 0) {
      //temp_IR = TEMP_IR_MIN;
      sign *= -1;
    }
    temp_sensor += 2.5;
    if (temp_sensor > TEMP_SENSOR_ALARM) {
      temp_sensor = TEMP_SENSOR_MIN;
    }
    temp_chip += 2.5;
    if (temp_chip > TEMP_CHIP_ALARM) {
      temp_chip = TEMP_CHIP_MIN;
    }
  #else 
    #if (DEBUG>1)
      Serial.println(F(" Updating all temps"));
    #endif
    temp_IR = getIRTemp();
    temp_sensor = getSensorTemp();
    temp_chip = getChipTemperature();
  #endif
}

void tempAlarmCheck() {
  #if (DEBUG>1)
    Serial.println(F(" Checking for temp alarms"));
  #endif
  if (temp_IR > TEMP_IR_ALARM) {
    declareError(IR_TEMPHIGH_ERR);
  } else if (temp_IR < TEMP_IR_MIN) {
    declareError(IR_TEMPLOW_ERR);
  }
  if (temp_sensor > TEMP_SENSOR_ALARM) {
    declareError(SENSOR_TEMPHIGH_ERR);
  } else if (temp_sensor < TEMP_SENSOR_MIN) {
    declareError(SENSOR_TEMPLOW_ERR);
  }
  if (temp_chip > TEMP_CHIP_ALARM) {
    declareError(CHIP_TEMPHIGH_ERR);
  } else if (temp_chip < TEMP_CHIP_MIN) {
    declareError(CHIP_TEMPLOW_ERR);
  }
  #if (DEBUG>1)
    Serial.println(F(" Check finished"));
  #endif
}

float getIRTemp() {
  #if (DEBUG>1)
    Serial.println(F(" Getting IR temp"));
  #endif
  // Issue start condition and indicate that we want to write
  i2c_start((MLX90614_I2CADDR << 1) | I2C_WRITE);
  // Send the byte indicating we will be reading OBJ1 temperature
  i2c_write(MLX90614_TOBJ1);    
  return readTempBytes();
}

float getSensorTemp() {
  #if (DEBUG>1)
    Serial.println(F(" Getting sensor temp"));
  #endif
  // Issue start condition and indicate that we want to write
  i2c_start((MLX90614_I2CADDR << 1) | I2C_WRITE);
  // Send the byte indicating we will be reading TA temperature
  i2c_write(MLX90614_TA);
  return readTempBytes();  
}

float readTempBytes() {  
  // Immediately request read
  i2c_rep_start((MLX90614_I2CADDR << 1) | I2C_READ);
  // As per datasheet, we get 3 bytes back
  // int16_t is used, makes later calcs simpler
  // PEC is not used due to low corruption likelyhood
  int16_t data_low = i2c_read(false);   // Read 1 byte and then send ack
  int16_t data_high = i2c_read(false);  // Read 1 byte and then send ack
  //uint8_t pec = i2c_read(true);       // Read 1 byte and then NACK (else can't reclaim bus control)
  i2c_read(true);
  i2c_stop();                           // Issue stop condition, release bus

  if (data_high & 0x0080) {
    // Error bit is 1!   
    #if (DEBUG > 2) 
      printBits(data_high);
      printBits(data_low);
      Serial.println(F(" IRSENSOR_EBYTE_ERR"));
    #endif
    declareError(IRSENSOR_EBYTE_ERR);
    return 0;
  } else if ((data_high == 0x0001) && (data_low == 0x0001)) {    
    #if (DEBUG > 2) 
      Serial.println(F(" IRSENSOR_NODATA_ERR"));
    #endif
    declareError(IRSENSOR_NODATA_ERR);
    return 0;
  } else {
    #if (DEBUG > 3) 
      Serial.print(F(" IR BITS|"));
      printBits(data_high);
      printBits(data_low);
      Serial.println("");
    #endif
    float tempFactor = 0.02; // 0.02 degrees per LSB (measurement resolution, see 8.7.2)
    float tempData = 0x0000; // zero out the data
    //int frac; // data past the decimal point
    
    // This masks off the error bit of the high byte, then moves it left 8 bits and adds the low byte.
    // But since we handle error separately, it is always 0 so no need anymore...
    // tempData = (float)(((data_high & 0x007F) << 8) + data_low);
    // tempData = (tempData * tempFactor)-0.01;
    tempData = (float)((data_high << 8) | data_low);
    tempData = (tempData * tempFactor)-273.15;
    return tempData;
    //float celcius = tempData - 273.15;
    //float fahrenheit = (celcius*1.8) + 32;
  }
}

