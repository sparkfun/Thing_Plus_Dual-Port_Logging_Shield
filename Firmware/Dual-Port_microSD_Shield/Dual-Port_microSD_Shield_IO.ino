// Dual-Port microSD Shield I2C interrupt routines

// I2C receiveEvent
// ================
void receiveEvent(int numberOfBytesReceived)
{
  if (numberOfBytesReceived > 0) // Check that we received some data (!) (hopefully redundant!)
  {
    receiveEventData.receiveEventRegister = Wire.read(); // Store the first byte so we know what to do during the next requestEvent

    if (numberOfBytesReceived > 1) // Did we receive more than one byte?
    {
      int i;
      for (i = 1; (i < numberOfBytesReceived) && (i < TINY_BUFFER_LENGTH); i++) // If we did, store it
      {
        receiveEventData.receiveEventBuffer[i - 1] = Wire.read();
      }
      i--;
      receiveEventData.receiveEventBuffer[i] = 0; // NULL-terminate the data
    }

    receiveEventData.receiveEventLength = (volatile byte)numberOfBytesReceived;
  }
}

// I2C requestEvent
// ================
void requestEvent()
{
  byte buff[2];
  switch (receiveEventData.receiveEventRegister)
  {
    case SFE_DUAL_SD_REGISTER_I2C_ADDRESS: // Does the user want to read the I2C address?
      Wire.write(eeprom_settings.i2cAddress);
      receiveEventData.receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Clear the event
      break;
    case SFE_DUAL_SD_REGISTER_FIRMWARE_VERSION: // Does the user want to read the firmware version?
      Wire.write(SFE_DUAL_SD_FIRMWARE_VERSION);
      receiveEventData.receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Clear the event
      break;
    case SFE_DUAL_SD_REGISTER_DEFAULT_MODE: // Does the user want to read the defaultMode?
      Wire.write(eeprom_settings.defaultMode);
      receiveEventData.receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Clear the event
      break;
    case SFE_DUAL_SD_REGISTER_UNKNOWN:
      // Nothing to do here - maybe debug?
      break;
    default:
      receiveEventData.receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Clear the event
      break;
  }
}
