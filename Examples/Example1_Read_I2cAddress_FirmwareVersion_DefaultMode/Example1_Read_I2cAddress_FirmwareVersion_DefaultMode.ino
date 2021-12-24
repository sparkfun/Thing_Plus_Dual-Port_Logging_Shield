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
  0x02 : Set / Get the defaultMode: 0x00 = Thing Plus / Arduino (SPI) mode; 0x01 = USB "memory stick" (SDIO) mode
  0x03 : Go into deep sleep (powewr down the microSD card too)
  0x04 : Wake from deep sleep and go into defaultMode

*/

#define SFE_DUAL_SD_MODE_SPI  0x00 // Thing Plus / Arduino (SPI) mode
#define SFE_DUAL_SD_MODE_SDIO 0x01 // USB "memory stick" (SDIO) mode

#include <Wire.h>

void setup()
{
  
  Serial.begin(115200); // Start the Serial port
  Serial.println(F("Dual-Port microSD Shield Code Example"));

  Wire.begin(); // Start I2C

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Get (read) the I2C Address
  //
  // This function is essentially redundant since we need to know the Shield's address before we can read from it!
  // But it is a useful sanity check.
  //
  // Returns 255 if the read failed
  
  uint8_t i2cAddress = dualPortMicroSDShield_getI2cAddress(0x51, Wire);

  if (i2cAddress == 255)
    Serial.println(F("An error has occurred! Could not communicate with the Shield!"));
  else
  {
    Serial.print(F("The Shield's I2C Address is 0x"));
    Serial.println(i2cAddress, HEX);
  }

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Get (read) the firmware version
  //
  // Returns 255 (0xFF) if the read failed
  // Version 1.0 = 0x10

  uint8_t firmwareVersion = dualPortMicroSDShield_getFirmwareVersion(0x51, Wire);

  if (firmwareVersion == 255)
    Serial.println(F("An error has occurred! Could not communicate with the Shield!"));
  else
  {
    Serial.print(F("The Shield's Firmware Version is v"));
    Serial.print(i2cAddress >> 4);
    Serial.print(F("."));
    Serial.println(i2cAddress & 0x0F);
  }
  
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Get (read) the default mode
  //
  // Returns 255 (0xFF) if the read failed
  // 0x00 = Thing Plus / Arduino (SPI) mode
  // 0x01 = USB "memory stick" (SDIO) mode

  uint8_t defaultMode = dualPortMicroSDShield_getDefaultMode(0x51, Wire);

  if (defaultMode == 255)
    Serial.println(F("An error has occurred! Could not communicate with the Shield!"));
  else
  {
    Serial.print(F("The Shield's Default Mode is "));
    if (defaultMode == SFE_DUAL_SD_MODE_SPI)
      Serial.println(F("SPI (Thing Plus / Arduino mode)"));
    else if (defaultMode == SFE_DUAL_SD_MODE_SDIO)
      Serial.println(F("SDIO (USB \"memory stick\" mode)"));
    else
      Serial.println(F("unknown! An error has occurred!"));      
  }
  
}

void loop()
{
  
  // Nothing to do here...

}
