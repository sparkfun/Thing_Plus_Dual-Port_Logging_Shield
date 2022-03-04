/*
  Dual-Port microSD Shield Functions
  By: Paul Clark
  Date: December 24th 2021

  These functions control the Dual-Port microSD Shield.

  The Shield's default I2C address is 0x51.

  Supported I2C commands / registers are:
  0x00 : Set / Get the I2C Address
  0x01 : Get the Firmware Version
  0x02 : Set / Get the defaultMode: 0x00 = Thing Plus / Arduino (SPI) mode; 0x01 = "thumb drive" (SDIO) mode
  0x03 : Go into deep sleep (powewr down the microSD card too)
  0x04 : Wake from deep sleep and go into defaultMode

*/

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Define the firmware command / register addresses:

const uint8_t SFE_DUAL_SD_REGISTER_I2C_ADDRESS        = 0x00; // byte     Read/Write: Stored in EEPROM
const uint8_t SFE_DUAL_SD_REGISTER_FIRMWARE_VERSION   = 0x01; // byte     Read only
const uint8_t SFE_DUAL_SD_REGISTER_DEFAULT_MODE       = 0x02; // byte     Read/Write: Stored in EEPROM
const uint8_t SFE_DUAL_SD_REGISTER_SLEEP              = 0x03; // Virtual write-only register: Go into deep sleep
const uint8_t SFE_DUAL_SD_REGISTER_WAKE               = 0x04; // Virtual write-only register: Wake from deep sleep and go into defaultMode

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Get (read) the I2C Address
//
// This function is essentially redundant since we need to know the Shield's address before we can read from it!
// But it is a useful sanity check.
//
// Returns 255 if the read failed

uint8_t dualPortMicroSDShield_getI2cAddress(uint8_t i2cAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(i2cAddress);
  wirePort.write(SFE_DUAL_SD_REGISTER_I2C_ADDRESS);
  wirePort.endTransmission();

  uint32_t bytesReceived = wirePort.requestFrom(i2cAddress, (uint8_t)1);  // Read one byte from the ATtiny841

  if (bytesReceived != 1) // If we did not receive exactly 1 byte, return an error (255)
    return (255);

  return (wirePort.read()); // Return the I2C Address
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Set (write) the I2C Address
//
// Returns 0 if the write was successful

uint8_t dualPortMicroSDShield_setI2cAddress(uint8_t newAddress, uint8_t i2cAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(i2cAddress);
  wirePort.write(SFE_DUAL_SD_REGISTER_I2C_ADDRESS);
  wirePort.write(newAddress);
  return (wirePort.endTransmission());
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Get (read) the firmware version
//
// Returns 255 (0xFF) if the read failed
// Version 1.0 = 0x10

uint8_t dualPortMicroSDShield_getFirmwareVersion(uint8_t i2cAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(i2cAddress);
  wirePort.write(SFE_DUAL_SD_REGISTER_FIRMWARE_VERSION);
  wirePort.endTransmission();

  uint32_t bytesReceived = wirePort.requestFrom(i2cAddress, (uint8_t)1);  // Read one byte from the ATtiny841

  if (bytesReceived != 1) // If we did not receive exactly 1 byte, return an error (255)
    return (255);

  return (wirePort.read()); // Return the firmware version
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Get (read) the Default Mode
//
// Returns 255 if the read failed

uint8_t dualPortMicroSDShield_getDefaultMode(uint8_t i2cAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(i2cAddress);
  wirePort.write(SFE_DUAL_SD_REGISTER_DEFAULT_MODE);
  wirePort.endTransmission();

  uint32_t bytesReceived = wirePort.requestFrom(i2cAddress, (uint8_t)1);  // Read one byte from the ATtiny841

  if (bytesReceived != 1) // If we did not receive exactly 1 byte, return an error (255)
    return (255);

  return (wirePort.read()); // Return the default mode
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Set (write) the Default Mode
//
// Returns 0 if the write was successful

uint8_t dualPortMicroSDShield_setDefaultMode(uint8_t mode, uint8_t i2cAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(i2cAddress);
  wirePort.write(SFE_DUAL_SD_REGISTER_DEFAULT_MODE);
  wirePort.write(mode);
  return (wirePort.endTransmission());
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Sleep
//
// Returns 0 if the command was successful

uint8_t dualPortMicroSDShield_sleep(uint8_t i2cAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(i2cAddress);
  wirePort.write(SFE_DUAL_SD_REGISTER_SLEEP);
  return (wirePort.endTransmission());
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Wake and go into Default Mode
//
// Returns 0 if the command was successful

uint8_t dualPortMicroSDShield_wake(uint8_t i2cAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(i2cAddress);
  wirePort.write(SFE_DUAL_SD_REGISTER_WAKE);
  return (wirePort.endTransmission());
}
