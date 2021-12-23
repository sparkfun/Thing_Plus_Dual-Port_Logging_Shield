// Dual-Port microSD Shield EEPROM storage and functions

struct struct_eeprom_settings {
  byte sizeOfSettings = 0; //sizeOfSettings **must** be the first entry and must be byte
  byte firmwareVersion = SFE_DUAL_SD_FIRMWARE_VERSION; // firmwareVersion **must** be the second entry
  byte i2cAddress = SFE_DUAL_SD_DEFAULT_I2C_ADDRESS;
  byte defaultMode = SFE_DUAL_SD_MODE_SPI; // Default to Thing Plus / Arduino (SPI) mode
  byte CRC;
} eeprom_settings;
