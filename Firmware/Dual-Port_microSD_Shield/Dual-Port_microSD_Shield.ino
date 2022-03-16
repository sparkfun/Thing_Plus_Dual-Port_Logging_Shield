/*
  Dual-Port microSD Shield Firmware
  By: Paul Clark
  Date: December 22nd 2021
  Version: 1.0

  This firmware runs on the ATtiny841 on the Dual-Port microSD Shield.

  Uses Spence Konde's ATTinyCore boards:
  https://github.com/SpenceKonde/ATTinyCore
  https://github.com/SpenceKonde/ATTinyCore/blob/master/Installation.md
  Add this URL to the Additional Boards Manager URLs in File\Preferences:
  http://drazzy.com/package_drazzy.com_index.json
  and then use the Boards Manager to add support for the ATtiny: http://boardsmanager/All#ATTinyCore

  The shield's default I2C address is 0x51. The address is stored in eeprom and can be changed.
  See the code example for more details.

  Set Board to ATtiny441/841 (No bootloader)
  Set Chip to ATtiny841
  Set Clock Source to 4MHz (internal, Vcc < 4.5)
  Set Pin Mapping to Clockwise
  Set Wire Mode to Slave Only
  Set millis()/micros() to Enabled
  Set Save EEPROM to EEPROM Not Retained
  Set BOD Level to BOD Enabled 2.7V
  Set BOD Mode (Active) to BOD Sampled Mode
  Set BOD Mode (Sleep) to BOD Sampled Mode

  Fuse byte settings (default to a 1MHz clock, the code will increase the clock to 4MHz):
  Low:  0b01000010 = 0x42 : divide clock by 8, no clock out, slowly rising power, 8MHz internal oscillator
  High: 0b11011101 = 0xDD : reset not disabled, no debug wire, SPI programming enabled, WDT not always on, EEPROM not preserved, BOD 2.7V
  Ext:  0b11101011 = 0xEB : ULP Osc 32kHz, BOD (Sleep) sampled, BOD (Active) sampled, no self-programming

  Pin Allocation (Physical pins are for the QFN package):
  D0:  Physical Pin 5  (PA0 / A0)          : microSD Power Enable
  D1:  Physical Pin 4  (PA1 / A1/TXD0)     : Serial TXD
  D2:  Physical Pin 3  (PA2 / A2/RXD0)     : Serial RXD
  D3:  Physical Pin 2  (PA3 / A3)          : SDIO (USB2241) Enable
  D4:  Physical Pin 1  (PA4 / A4/SCK/SCL)  : I2C SCL
  D5:  Physical Pin 20 (PA5 / A5/CIPO)     : POCI - connected to pin 1 of the ISP header
  D6:  Physical Pin 16 (PA6 / A6/COPI/SDA) : I2C SDA
  D7:  Physical Pin 15 (PA7 / A7)          : Thing Plus 3V3 divided by 2
  D8:  Physical Pin 14 (PB2 / A8)          : USB 5V divided by 2
  D11: Physical Pin 13 (PB3 / A9/!RESET)   : !RESET
  D9:  Physical Pin 12 (PB1 / A10/INT0)    : microSD SCK (Interrupt 0)
  D10: Physical Pin 11 (PB0 / A11)         : SPI Buffer Enable

  There is a huge amount we could do with this firmware, but we have decided to Keep It Simple!

  When the ATtimy comes out of reset, it will:
  Delay for 500ms
  Measure the Thing Plus 3V3 rail
  Measure the USB 5V rail
  If USB power is present and 3V3 is not, the ATtiny will power on the USB2241 and go into "thumb drive" (SDIO) mode
  If 3V3 power is present and USB power is not, the ATtiny will go into Thing Plus / Arduino (SPI) mode
  If both power rails are present, the ATtiny will go into whichever mode is defined by defaultMode in EEPROM

  The user can command the ATtiny to power off the microSD card and go into deep sleep by sending command 0x03

  The user can change modes by sending command 0x04 or 0x05
  Command 0x04 will select Thing Plus / Arduino (SPI) mode
  Command 0x05 will select "thumb drive" (SDIO) mode

  Commands 0x04 and 0x05 can also be used to wake the ATtiny from deep sleep

  Simple diagnostics are output on the Serial TX pin at power up. (9600 baud)

  To Keep It Simple:
  The microSD SCK interrupt (INT0) is not used. (This could be used in the future to monitor microSD card activity in either mode)

  Supported I2C commands / registers are:
  0x00 : Set / Get the I2C Address
  0x01 : Get the Firmware Version
  0x02 : Set / Get the defaultMode: 0x00 = Thing Plus / Arduino (SPI) mode; 0x01 = "thumb drive" (SDIO) mode
  0x03 : Go into deep sleep (power down the microSD card too)
  0x04 : Wake from deep sleep (if required). Go into Thing Plus / Arduino (SPI) mode
  0x05 : Wake from deep sleep (if required). Go into "thumb drive" (SDIO) mode

*/

#include <Wire.h>

#include <EEPROM.h>

#include <avr/sleep.h> //Needed for sleep_mode
//#include <avr/power.h> //Needed for powering down perihperals such as the ADC
//#include <avr/wdt.h>

#include "Dual-Port_microSD_Shield_Constants.h"
#include "Dual-Port_microSD_Shield_EEPROM.h"

//Diagnostics
#define SERIAL_DIAGNOSTICS // Uncomment this line to enable diagnostics on serial at 9600 baud

//Digital pins
const byte SPI_EN = 10; // SPI Enable: pull high to enable SPI buffer, pull low to disable
const byte MICROSD_CLK = 9; // MICROSD_CLK: could be used as an interrupt (INT0) to indicate SD card activity
const byte SDA_PIN = 6;
const byte SCL_PIN = 4;
const byte SDIO_EN = 3; // SDIO Enable: pull high to enable the USB2241 and its buffers, pull low to disable
const byte MICROSD_PWR_EN = 0; // microSD Power Enable: pull low to enable power for the microSD card, pull high or leave floating to disable

//Analog pins
const byte V_USB_DIV_2 = 8; // A8
const byte V_3V3_DIV_2 = 7; // A7

//Define the ON and OFF states for each pin
#define SPI_EN__ON   HIGH // SPI Enable: pull high to enable SPI buffer
#define SPI_EN__OFF  LOW  // SPI Enable: pull low to disable
#define SDIO_EN__ON   HIGH // SDIO Enable: pull high to enable the USB2241 and its buffers
#define SDIO_EN__OFF  LOW  // SDIO Enable: pull low to disable
#define MICROSD_PWR_EN__ON   LOW  // microSD Power Enable: pull low to enable power for the microSD card
#define MICROSD_PWR_EN__OFF  HIGH // microSD Power Enable: pull high or leave floating to disable

//Global variables
#define TINY_BUFFER_LENGTH 16
struct {
  volatile byte receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Most recent receive event register address
  volatile byte receiveEventBuffer[TINY_BUFFER_LENGTH]; // byte array to store the data from the receive event
  volatile byte receiveEventLength = 0; // Indicates how many data bytes were received
} receiveEventData;
volatile bool sleepNow = false;

void setup()
{
  //Just in case, make sure the system clock is set to 4MHz (8MHz divided by 2)
  cli(); // Disable interrupts
  CCP = 0xD8; // Write signature (0xD8) to the Configuration Change Register
  CLKPR = 1; //Set clock prescaler CLKPS bits to 1 == divide by 2
  sei(); // Enable interrupts

  // Make sure MICROSD_CLK (D9 / INT0 / XTAL2) is an input.
  // This seems critical!? Without this, badness happens when the USB2241 starts up in SDIO mode.
  pinMode(MICROSD_CLK, INPUT);

  // Disable the buffers, microSD card and the USB2241
  digitalWrite(SPI_EN, SPI_EN__OFF);
  pinMode(SPI_EN, OUTPUT);
  digitalWrite(SPI_EN, SPI_EN__OFF);
  
  digitalWrite(SDIO_EN, SDIO_EN__OFF);
  pinMode(SDIO_EN, OUTPUT);
  digitalWrite(SDIO_EN, SDIO_EN__OFF);
  
  digitalWrite(MICROSD_PWR_EN, MICROSD_PWR_EN__OFF);
  pinMode(MICROSD_PWR_EN, OUTPUT);
  digitalWrite(MICROSD_PWR_EN, MICROSD_PWR_EN__OFF);

  disableWDT(); // Make sure the WatchDog Timer is disabled

  bool eepromReInit = false;
  if (!loadEepromSettings()) // Load the settings from eeprom
  {
    eepromReInit = true;
    initializeEepromSettings(); // Initialize them if required
  }

#ifdef SERIAL_DIAGNOSTICS
  // Send diagnostics over serial
  Serial.begin(9600);
  Serial.println(F("Dual-Port microSD Shield"));
  if (eepromReInit)
    Serial.println(F("Bad EEPROM data was detected on start-up. EEPROM reinitialized"));
  Serial.print(F("Firmware version "));
  Serial.print(eeprom_settings.firmwareVersion >> 4);
  Serial.print(F("."));
  Serial.println(eeprom_settings.firmwareVersion & 0x0F);
  Serial.print(F("I2C Address 0x"));
  Serial.println(eeprom_settings.i2cAddress, HEX);
  Serial.print(F("Default Mode "));
  if (eeprom_settings.defaultMode == SFE_DUAL_SD_MODE_SPI)
    Serial.println(F("SPI"));
  else
    Serial.println(F("SDIO"));
#endif

  //Delay for 500ms to let the voltage rails stabilize
  delay(500);

  //Read the USB and 3V3 voltages and decide which mode to go into
  float voltageUSB = readUSBvoltage();
  float voltage3V3 = read3V3voltage();

#ifdef SERIAL_DIAGNOSTICS
  Serial.print(F("Voltages: "));
  Serial.print(voltageUSB);
  Serial.print(F(","));
  Serial.println(voltage3V3);
#endif  

  //If USB power is present and 3V3 is not, the ATtiny will power on the USB2241 and go into "thumb drive" (SDIO) mode
  if ((voltageUSB > 4.0) && (voltage3V3 < 2.7))
  {
    sdioMode();
  }
  //If 3V3 power is present and USB power is not, the ATtiny will go into Thing Plus / Arduino (SPI) mode
  else if ((voltageUSB < 4.0) && (voltage3V3 > 2.7))
  {
    spiMode();
  }
  //If both power rails are present, the ATtiny will go into whichever mode is defined by EEPROM defaultMode
  else
  {
    if (eeprom_settings.defaultMode == SFE_DUAL_SD_MODE_SPI)
      spiMode();
    else
      sdioMode();
  }

  //Begin listening on I2C
  startI2C(true); // Skip Wire.end the first time around
}

void loop()
{
  //Check if a receive event has taken place
  if (receiveEventData.receiveEventLength > 0)
  {
    //Clear the event as early as possible so that the next event can arrive while (e.g.) the eeprom settings are being updated
    switch (receiveEventData.receiveEventRegister)
    {
      case SFE_DUAL_SD_REGISTER_I2C_ADDRESS: // Does the user want to change the I2C address?
        // receiveEventLength will be 1 if the user just wants to read the address
        if (receiveEventData.receiveEventLength == 2) // Data should be: register; new address
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
          eeprom_settings.i2cAddress = receiveEventData.receiveEventBuffer[0] & 0x7F; // Update the I2C address in eeprom. Limit to 7 bits
          saveEepromSettings(); // Update the address in eeprom
          startI2C(false); // Restart I2C comms
        }
        else
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
        }
        break;
      case SFE_DUAL_SD_REGISTER_FIRMWARE_VERSION: // Does the user want to read the firmware version?
        receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
        break;
      case SFE_DUAL_SD_REGISTER_DEFAULT_MODE: // Does the user want to change the defaultMode?
        // receiveEventLength will be 1 if the user just wants to read the default mode
        if (receiveEventData.receiveEventLength == 2) // Data should be: register; new address
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
          eeprom_settings.defaultMode = receiveEventData.receiveEventBuffer[0] & 0x01; // Update the defaultMode in eeprom. Limit to 1 bit
          saveEepromSettings(); // Update the defaultMode in eeprom
        }
        else
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
        }
        break;
      case SFE_DUAL_SD_REGISTER_SLEEP: // Does the user want to sleep?
        receiveEventData.receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Clear the receive event register - this one is write only
        if (receiveEventData.receiveEventLength == 1) // Should be a single byte
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
          sleepNow = true; // Go to sleep
        }
        else
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
        }
        break;
      case SFE_DUAL_SD_REGISTER_SPI_MODE: // Does the user want to wake (if required) and change mode?
        receiveEventData.receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Clear the receive event register - this one is write only
        if (receiveEventData.receiveEventLength == 1) // Should be a single byte
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event

          powerOffSD(); // Turn off the buffers and microSD card. They will already be off if the ATtiny was asleep

          delay(500);
          
          spiMode(); // Go into SPI mode
        }
        else
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
        }
        break;
      case SFE_DUAL_SD_REGISTER_SDIO_MODE: // Does the user want to wake (if required) and change mode?
        receiveEventData.receiveEventRegister = SFE_DUAL_SD_REGISTER_UNKNOWN; // Clear the receive event register - this one is write only
        if (receiveEventData.receiveEventLength == 1) // Should be a single byte
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event

          powerOffSD(); // Turn off the buffers and microSD card. They will already be off if the ATtiny was asleep

          delay(500);
          
          sdioMode(); // Go into SDIO mode 
        }
        else
        {
          receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
        }
        break;
      case SFE_DUAL_SD_REGISTER_UNKNOWN:
      default:
        receiveEventData.receiveEventLength = 0; // Clear the event - ready for the next event
        break;
    }    
  }

  // Sleep
  if (sleepNow)
  {
    powerOffSD(); // Turn off the buffers and microSD card

    byte adcsra = ADCSRA; //save ADCSRA (ADC control and status register A)
    ADCSRA &= ~_BV(ADEN); //disable ADC by clearing the ADEN bit
    
    byte acsr0a = ACSR0A; //save ACSR0A (Analog Comparator 0 control and status register)
    ACSR0A &= ~_BV(ACIE0); //disable AC0 interrupt
    ACSR0A |= _BV(ACD0); //disable ACO by setting the ACD0 bit
    
    byte acsr1a = ACSR1A; //save ACSR1A (Analog Comparator 1 control and status register)
    ACSR1A &= ~_BV(ACIE1); //disable AC1 interrupt
    ACSR1A |= _BV(ACD1); //disable AC1 by setting the ACD1 bit
    
    byte prr = PRR; // Save the power reduction register
    // Disable the ADC, USART1, SPI, Timer1 and Timer2
    // (Leave TWI, USART0 and Timer0 enabled)
    PRR |= _BV(PRADC) | _BV(PRUSART1) | _BV(PRSPI) | _BV(PRTIM1) | _BV(PRTIM2); 
    
    byte mcucr = MCUCR; // Save the MCU Control Register
    // Set Sleep Enable (SE=1), Power-down Sleep Mode (SM1=1, SM0=0), INT0 Change (ISC01=0, ISC00=1)
    MCUCR = _BV(SE) | _BV(SM1) | _BV(ISC00);

    sleep_cpu(); //go to sleep
    
    MCUCR = mcucr; // Restore the MCU control register
    PRR = prr; // Restore the power reduction register
    ACSR1A = acsr1a; // Restore ACSR1A    
    ACSR0A = acsr0a; // Restore ACSR0A    
    ADCSRA = adcsra; //Restore ADCSRA

    sleepNow = false; // Clear sleepNow
  }
}

//Begin listening on I2C bus as I2C peripheral using the global I2C_ADDRESS
void startI2C(bool skipWireEnd)
{
  if (skipWireEnd == false)
    Wire.end(); //Before we can change addresses we need to stop

  Wire.begin(eeprom_settings.i2cAddress); //Do the Wire.begin using the defined I2C_ADDRESS

  //The connections to the interrupts are severed when a Wire.begin occurs. So re-declare them.
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

//Measure the USB voltage using VCC as the reference
float readUSBvoltage()
{
  analogReference(SFE_DUAL_SD_ADC_REFERENCE_VCC); //Use VCC as the reference
  
  // "The first ADC conversion result after switching reference voltage source may be inaccurate,
  //  and the user is advised to discard this result."
  analogRead(0x80 | V_USB_DIV_2); //Read the analog voltage and discard the result
  
  uint16_t result = 0;
  for (byte x = 0; x < 8; x++)
  {
    result += analogRead(0x80 | V_USB_DIV_2); //Read the analog voltage
    noIntDelay(1);
  }
  
  float voltage = (float)(result >> 3); // Divide result by 8. Convert to float
  voltage /= 1023.0; // Convert to fraction of VCC
  voltage *= 3.3; // Convert to voltage (multiply by VCC)
  voltage *= 2.0; // Correct for the voltage divider
  
  return (voltage);
}

//Measure the 3V3 voltage using VCC as the reference
float read3V3voltage()
{
  analogReference(SFE_DUAL_SD_ADC_REFERENCE_VCC); //Use VCC as the reference
  
  // "The first ADC conversion result after switching reference voltage source may be inaccurate,
  //  and the user is advised to discard this result."
  analogRead(0x80 | V_3V3_DIV_2); //Read the analog voltage and discard the result
  
  uint16_t result = 0;
  for (byte x = 0; x < 8; x++)
  {
    result += analogRead(0x80 | V_3V3_DIV_2); //Read the analog voltage
    noIntDelay(1);
  }
  
  float voltage = (float)(result >> 3); // Divide result by 8. Convert to float
  voltage /= 1023.0; // Convert to fraction of VCC
  voltage *= 3.3; // Convert to voltage (multiply by VCC)
  voltage *= 2.0; // Correct for the voltage divider
  
  return (voltage);
}

//Configure the pins for Thing Plus / Arduino (SPI) mode
void spiMode()
{
  digitalWrite(MICROSD_PWR_EN, MICROSD_PWR_EN__ON); // Enable power for the microSD card
  delay(250);
  digitalWrite(SDIO_EN, SDIO_EN__OFF); // Disable the USB2241 and the SDIO buffers
  delay(100);
  digitalWrite(SPI_EN, SPI_EN__ON); // Enable the SPI buffer
  delay(100);
}

//Configure the pins for "thumb drive" (SDIO) mode
void sdioMode()
{
  digitalWrite(MICROSD_PWR_EN, MICROSD_PWR_EN__ON); // Enable power for the microSD card
  delay(250);
  digitalWrite(SPI_EN, SPI_EN__OFF); // Disable the SPI buffer
  delay(100);
  digitalWrite(SDIO_EN, SDIO_EN__ON); // Enable the USB2241 and the SDIO buffers
  delay(100);
}

//Turn off the buffers and microSD card
void powerOffSD()
{
  digitalWrite(SDIO_EN, SDIO_EN__OFF); // Disable the USB2241 and the SDIO buffers
  delay(50);
  digitalWrite(SPI_EN, SPI_EN__OFF); // Disable the SPI buffer
  delay(50);
  digitalWrite(MICROSD_PWR_EN, MICROSD_PWR_EN__OFF); // Disable power for the microSD card
  delay(50);
}

// Disable the WDT
void disableWDT()
{
  cli(); // Disable interrupts
  CCP = 0xD8; // Write signature (0xD8) to the Configuration Change Register
  // Within the next four clock cycles, write a logic 0 to WDE.
  WDTCSR = 0x00; // Disable the Watchdog and the interrupt and clear the prescaler.
  sei(); // Enable interrupts
}

//Software delay. Does not rely on internal timers.
void noIntDelay(byte amount)
{
  for (volatile byte y = 0 ; y < amount ; y++)
  {
    //ATtiny84 at 4MHz
    for (volatile unsigned int x = 0 ; x < 202 ; x++) //1ms at 4MHz
    {
      __asm__("nop\n\t");
    }
  }
}
