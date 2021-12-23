// Dual-Port microSD Shield EEPROM storage and functions

// Load the settings from eeprom into RAM
// Return true if settings are loaded correctly and valid, false otherwise
bool loadEepromSettings()
{
  bool eepromSettingsValid = true;

  byte settingsSize = 0;
  EEPROM.get(0, settingsSize); // Read sizeOfSettings from eeprom
  if (settingsSize != sizeof(eeprom_settings)) // Check that the size is correct
    eepromSettingsValid = false;

  byte settingsVersion = 0;
  EEPROM.get(sizeof(byte), settingsVersion); // Read firmwareVersion from eeprom
  if (settingsVersion != SFE_DUAL_SD_FIRMWARE_VERSION)
    eepromSettingsValid = false;

  EEPROM.get(0, eeprom_settings); // Read the settings from eeprom into RAM

  if (!checkEepromSettingsCRC()) // Check if the CRC is valid now that the settings are in RAM
    eepromSettingsValid = false;

  return (eepromSettingsValid);
}

//Save the settings into eeprom. Update sizeOfSettings and the CRC first.
void saveEepromSettings()
{
  eeprom_settings.sizeOfSettings = sizeof(eeprom_settings); // Update eeprom_settings
  eeprom_settings.CRC = computeCRC8((byte*)&eeprom_settings, eeprom_settings.sizeOfSettings - 1);
  EEPROM.put(0, eeprom_settings);
}

//Initialize eeprom settings to the defaults
void initializeEepromSettings()
{
  struct_eeprom_settings defaultSettings; //Load the default settings
  memcpy(&eeprom_settings, &defaultSettings, sizeof(defaultSettings)); //Overwrite eeprom_settings with the defaults
  saveEepromSettings(); //Update the settings in eeprom - also updates sizeOfSettings and the CRC
}

// Check if the settings loaded from eeprom are valid
bool checkEepromSettingsCRC()
{
  return(computeCRC8((byte*)&eeprom_settings, eeprom_settings.sizeOfSettings - 1) == eeprom_settings.CRC);
}

// Compute the CRC8 for the provided data
byte computeCRC8(byte *data, byte len)
{
  byte crc = 0xFF; //Init with 0xFF

  for (byte x = 0; x < len; x++)
  {
    crc ^= data[x]; // XOR-in the next input byte

    for (byte i = 0; i < 8; i++)
    {
      if ((crc & 0x80) != 0)
        crc = (byte)((crc << 1) ^ 0x31);
      else
        crc <<= 1;
    }
  }

  return crc; //No output reflection
}
