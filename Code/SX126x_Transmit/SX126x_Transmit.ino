/*
   RadioLib SX126x Transmit Example

   This example transmits packets using SX1262 LoRa radio module.
   Each packet contains up to 256 bytes of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)

   Other modules from SX126x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

// include the library
#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

// SX1262 has the following connections:
// NSS pin:   15
// DIO1 pin:  4
// NRST pin:  16
// BUSY pin:  17

SPIClass mySpi(VSPI);
SPISettings spiSettings(2000000, MSBFIRST, SPI_MODE0);
SX1262 radio = new Module(5, 4, 16, 17, mySpi, spiSettings);

// SPIClass *loraSPI = new SPIClass(HSPI);
// SX1262 radio = new Module(5, 4, 16, 17, *loraSPI);
//  or using RadioShield
//  https://github.com/jgromes/RadioShield
// SX1262 radio = RadioShield.ModuleA;

// or using CubeCell
// SX1262 radio = new Module(RADIOLIB_ONBOARD_MODULE);

void setup()
{
  Serial.begin(9600);

  mySpi.begin();

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));  
  //  int16_t begin(float freq = 868.0, float bw = 125.0, uint8_t sf = 9, uint8_t cr = 7, uint8_t syncWord = RADIOLIB_SX126X_SYNC_WORD_PRIVATE, int8_t power = 10, uint16_t preambleLength = 8, float tcxoVoltage = 1.6, bool useRegulatorLDO = false);
  // https://www.rfwireless-world.com/calculators/LoRa-Data-Rate-Calculator.html

  int state = radio.begin(868.39, 125.0, 12, 7, 0x4A, 14);  

  /*              
    Mode	Bitrate (bits/sec)	Max payload size (bytes)
    SF7/125kHz	5470	222
    SF8/125kHz	3125	222
    SF9/125kHz	1760	115
    SF10/125kHz	980	51
    SF11/125kHz	440	51
    SF12/125kHz	250	51
    SF7/250kHz	11000	222
  */
  // begin(868.0, 125.0, 8, 5);

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true)
      ;
  }

  // some modules have an external RF switch
  // controlled via two pins (RX enable, TX enable)
  // to enable automatic control of the switch,
  // call the following method
  // RX enable:   4
  // TX enable:   5
  
    radio.setRfSwitchPins(25, 25);  
  
}

void loop()
{
  sendHello();
  // wait for a second before transmitting again
  delay(1000);
}

void receiveText()
{
}

