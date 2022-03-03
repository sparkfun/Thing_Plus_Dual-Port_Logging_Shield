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

#include <SD.h>

//const int chipSelect = 10; // ** Change this to match the chip slect pin on your board **
#define chipSelect A5 // ** Change this to match the chip slect pin on your board **

File root;

void setup()
{
  
  Serial.begin(115200); // Start the Serial port
  Serial.println(F("Dual-Port microSD Shield Code Example"));

  delay(1000); // Let the shield start up - it takes a full second

  Wire.begin(); // Start I2C

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Get (read) the default mode
  //
  // Returns 255 (0xFF) if the read failed
  // 0x00 = Thing Plus / Arduino (SPI) mode
  // 0x01 = USB "thumb drive" (SDIO) mode

  uint8_t defaultMode = dualPortMicroSDShield_getDefaultMode(0x51, Wire);

  if (defaultMode == 255)
    Serial.println(F("An error has occurred! Could not communicate with the Shield!"));
  else
  {
    Serial.print(F("The Shield's Default Mode is "));
    if (defaultMode == SFE_DUAL_SD_MODE_SPI)
      Serial.println(F("SPI (Thing Plus / Arduino mode)"));
    else if (defaultMode == SFE_DUAL_SD_MODE_SDIO)
      Serial.println(F("SDIO (USB \"thumb drive\" mode)"));
    else
      Serial.println(F("unknown! An error has occurred!"));      
  }

  if (defaultMode == SFE_DUAL_SD_MODE_SPI)
  {
    // Use the SD Library to print the directory of files on SD card
    // https://www.arduino.cc/en/Tutorial/LibraryExamples/Listfiles
  
    Serial.println(F("Initializing SD card..."));
  
    if (!SD.begin(chipSelect))
    {
      Serial.println("microSD card initialization failed. Maybe because the power has not been cycled?");
    }
    else
    {
      Serial.println("Initialization complete.");
    
      root = SD.open("/");
    
      printDirectory(root, 0);
    }
  
    delay(2000); // Be kind. Wait two seconds
  }
}

void loop()
{
  
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Set the default mode to USB "thumb drive" (SDIO) mode
  //
  // Returns 0 if the write was successful

  Serial.println(F("Setting the default mode to SDIO (USB \"thumb drive\" mode)"));

  byte result = dualPortMicroSDShield_setDefaultMode(SFE_DUAL_SD_MODE_SDIO, 0x51, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ;
  }

  delay(5); // This delay is important. Wait 5ms after changing the mode before sending the sleep command. Otherwise the sleep command can be missed.

  // To change modes, we need to put the Shield to sleep and then wake it again

  Serial.println(F("Putting the Shield to sleep"));

  result = dualPortMicroSDShield_sleep(0x51, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ;
  }

  delay(2000); // Be kind. Wait two seconds
  
  Serial.println(F("Waking the Shield"));

  result = dualPortMicroSDShield_wake(0x51, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ;
  }

  Serial.println(F("The shield should now appear as a USB device"));

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  Serial.print(F("Waiting 30 seconds"));
  for (int i = 0; i < 30; i++)
  {
    delay(1000);
    Serial.print(F("."));
  }
  Serial.println();

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Set the default mode to Thing Plus / Arduino (SPI) mode
  //
  // Returns 0 if the write was successful

  Serial.println(F("Setting the default mode to SPI (Thing Plus / Arduino mode)"));

  result = dualPortMicroSDShield_setDefaultMode(SFE_DUAL_SD_MODE_SPI, 0x51, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ;
  }

  delay(5); // This delay is important. Wait 5ms after changing the mode before sending the sleep command. Otherwise the sleep command can be missed.

  // To change modes, we need to put the Shield to sleep and then wake it again

  Serial.println(F("Putting the Shield to sleep"));

  result = dualPortMicroSDShield_sleep(0x51, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ;
  }

  delay(2000); // Be kind. Wait two seconds
  
  Serial.println(F("Waking the Shield"));

  result = dualPortMicroSDShield_wake(0x51, Wire);

  if (result != 0)
  {
    Serial.println(F("An error has occurred! Could not communicate with the Shield! Freezing..."));
    while (1)
      ;
  }

  delay(2000); // Be kind. Wait two seconds for the shield to wake up

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // Use the SD Library to print the directory of files on SD card
  // https://www.arduino.cc/en/Tutorial/LibraryExamples/Listfiles

  Serial.println(F("Initializing SD card..."));

  if (!SD.begin(chipSelect))
  {
    Serial.println("microSD card initialization failed! Freezing...");
    while (1)
      ;
  }

  Serial.println("Initialization complete.");

  root = SD.open("/");

  printDirectory(root, 0);

  delay(1000); // Be kind. Wait one second

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

} // /loop

void printDirectory(File dir, int numTabs) // https://www.arduino.cc/en/Tutorial/LibraryExamples/Listfiles
{

  while (true)
  {
    File entry =  dir.openNextFile();

    if (! entry)
    {
      // no more files
      break;
    }

    for (uint8_t i = 0; i < numTabs; i++)
    {
      Serial.print('\t');
    }

    Serial.print(entry.name());

    if (entry.isDirectory())
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    }
    else
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }

    entry.close();
  }
  
} // /printDirectory
