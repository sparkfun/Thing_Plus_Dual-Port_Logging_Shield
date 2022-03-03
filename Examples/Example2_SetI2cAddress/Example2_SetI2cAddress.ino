/*
  Dual-Port microSD Shield Code Example
  By: Paul Clark
  Date: December 24th 2021

  This code example demonstrates how to use the Dual-Port microSD Shield.

  Communication with the Shield is so simple, it does not have a full Arduino Library.
  All the functions you need are included in Dual-Port_microSD_Shield_Functions.ino.

  The Shield's default I2C address is 0x51.

  Supported I2C commands / registers are:
  0x00 : Set / Get the I2C Address
  0x01 : Get the Firmware Version
  0x02 : Set / Get the defaultMode: 0x00 = Thing Plus / Arduino (SPI) mode; 0x01 = USB "thumb drive" (SDIO) mode
  0x03 : Go into deep sleep (powewr down the microSD card too)
  0x04 : Wake from deep sleep and go into defaultMode

*/

#define SFE_DUAL_SD_MODE_SPI  0x00 // Thing Plus / Arduino (SPI) mode
#define SFE_DUAL_SD_MODE_SDIO 0x01 // USB "thumb drive" (SDIO) mode

#include <Wire.h>

void setup()
{
  
  Serial.begin(115200); // Start the Serial port
  Serial.println(F("Dual-Port microSD Shield Code Example"));

  delay(1000); // Let the shield start up - it takes a full second

  Wire.begin(); // Start I2C

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Get (read) the I2C Address
  //
  // This function is essentially redundant since we need to know the Shield's address before we can read from it!
  // But it proves the Shield is connected and communicating.
  //
  // Returns 255 if the read failed
  
  uint8_t i2cAddress = dualPortMicroSDShield_getI2cAddress(0x51, Wire);

  if (i2cAddress == 255)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else
  {
    Serial.print(F("The Shield's I2C Address is 0x"));
    Serial.println(i2cAddress, HEX);
  }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Set (write) the I2C Address
  //
  // Returns 0 if the write was successful

  Serial.println(F("Setting the Shield's I2C Address to 0x52"));
  
  byte result = dualPortMicroSDShield_setI2cAddress(0x52, 0x51, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else
  {
    Serial.println(F("The Shield's I2C Address has been changed"));
  }

  delay(1000); // Allow time for the shield to restart its I2C interface

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Get (read) the I2C Address
  
  i2cAddress = dualPortMicroSDShield_getI2cAddress(0x52, Wire);

  if (i2cAddress == 255)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else
  {
    Serial.print(F("The Shield's I2C Address is 0x"));
    Serial.println(i2cAddress, HEX);
  }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Set (write) the I2C Address

  Serial.println(F("Setting the Shield's I2C Address back to 0x51"));
  
  result = dualPortMicroSDShield_setI2cAddress(0x51, 0x52, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else
  {
    Serial.println(F("The Shield's I2C Address has been changed"));
  }

  delay(1000); // Allow time for the shield to restart its I2C interface

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Get (read) the I2C Address
  
  i2cAddress = dualPortMicroSDShield_getI2cAddress(0x51, Wire);

  if (i2cAddress == 255)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ; // Do nothing more
  }
  else
  {
    Serial.print(F("The Shield's I2C Address is 0x"));
    Serial.println(i2cAddress, HEX);
  }

}

void loop()
{
  
  // Nothing to do here...

}
