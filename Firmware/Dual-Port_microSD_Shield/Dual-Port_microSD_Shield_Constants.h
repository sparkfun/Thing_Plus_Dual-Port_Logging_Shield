// Dual-Port microSD Shield EEPROM constants

//Define the firmware version
#define SFE_DUAL_SD_FIRMWARE_VERSION            0x10 // v1.0

//Define the ATtiny841's default I2C address (unshifted)
#define SFE_DUAL_SD_DEFAULT_I2C_ADDRESS         0x51

//Define the modes
#define SFE_DUAL_SD_MODE_SPI                    0x00 // Thing Plus / Arduino (SPI) mode
#define SFE_DUAL_SD_MODE_SDIO                   0x01 // "memory stick" (SDIO) mode

//Define the firmware register addresses:
#define SFE_DUAL_SD_REGISTER_I2C_ADDRESS        0x00 // byte     Read/Write: Stored in EEPROM
#define SFE_DUAL_SD_REGISTER_FIRMWARE_VERSION   0x01 // byte     Read only
#define SFE_DUAL_SD_REGISTER_DEFAULT_MODE       0x02 // byte     Read/Write: Stored in EEPROM
#define SFE_DUAL_SD_REGISTER_SLEEP              0x03 // Virtual write-only register: Go into deep sleep
#define SFE_DUAL_SD_REGISTER_WAKE               0x04 // Virtual write-only register: Wake from deep sleep and go into defaultMode
#define SFE_DUAL_SD_REGISTER_UNKNOWN            0xFF

//ADC Reference (with AREF disconnected)
#define SFE_DUAL_SD_ADC_REFERENCE_VCC           0
#define SFE_DUAL_SD_ADC_REFERENCE_1V1           1
#define SFE_DUAL_SD_ADC_REFERENCE_2V2           2
#define SFE_DUAL_SD_ADC_REFERENCE_4V096         3
