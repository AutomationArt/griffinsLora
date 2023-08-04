/*
   RadioLib SX126x Receive Example

   This example listens for LoRa transmissions using SX126x Lora modules.
   To successfully receive data, the following settings have to be the same
   on both transmitter and receiver:
    - carrier frequency
    - bandwidth
    - spreading factor
    - coding rate
    - sync word
    - preamble length

   Other modules from SX126x family can also be used.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "esp_log.h"    
// #define TEST

#define i2c_Address 0x3c // initialize with the I2C addr 0x3C Typically eBay OLED's
// #define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    //   QT-PY / XIAO
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define FreeButton 36
#define LedPin 2

struct LoraDataBase
{
  String message;
  float rssi;
  float snr;
};

LoraDataBase dBase[40];

int dBase_add(String text, float rssi, float snr)
{
  int size = sizeof(dBase) / sizeof(LoraDataBase);

  for (int i = size - 1; i > 0; i--)
  {
    dBase[i].message = dBase[i - 1].message;
    dBase[i].snr = dBase[i - 1].snr;
    dBase[i].rssi = dBase[i - 1].rssi;
  }
  dBase[0].message = text;
  dBase[0].snr = snr;
  dBase[0].rssi = rssi;
  return 0;
}

int draw_dBase()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  for (int i = 7; i >= 0; i--)
  {
    display.setCursor(0, i * 7);
    dBase[i].message = dBase[i].message;  
    display.println(dBase[i].message);
  }
  display.display();

  return 0;
}

bool handleButton()
{
  static bool previousButtonState = LOW;
  bool currentButtonState = analogRead(FreeButton);
  if (currentButtonState >3000)
  {
    ESP_LOGD("Button", "True");
    return true;
  }       
  else
  {   
    ESP_LOGD("Button", "False");
    return false;
  }
  return false;
} 

// SX1262 has the following connections:
// NSS pin:   5
// DIO1 pin:  13
// NRST pin:  16
// BUSY pin:  17
// SX1262 radio = new Module(10, 2, 3, 9);

SPIClass mySpi(VSPI);
SPISettings spiSettings(2000000, MSBFIRST, SPI_MODE0);
SX1262 radio = new Module(5, 4, 16, 17, mySpi, spiSettings);

// or using RadioShield
// https://github.com/jgromes/RadioShield
// SX1262 radio = RadioShield.ModuleA;

// or using CubeCell
// SX1262 radio = new Module(RADIOLIB_ONBOARD_MODULE);

void sendHello()
{

  Serial.print(F("[SX1262] Transmitting packet ... "));
  // you can transmit C-string or Arduino string up to
  // 256 characters long
  // NOTE: transmit() is a blocking method!
  //       See example SX126x_Transmit_Interrupt for details
  //       on non-blocking transmission method.

  String transText = String(millis());
  Serial.print("Send: ");
  Serial.println(transText);
  int state = radio.transmit(transText);

  // you can also transmit byte array up to 256 bytes long
  /*
    byte byteArr[] = {0x01, 0x23, 0x45, 0x56, 0x78, 0xAB, 0xCD, 0xEF};
    int state = radio.transmit(byteArr, 8);
  */
  if (state == RADIOLIB_ERR_NONE)
  {
    // the packet was successfully transmitted
    Serial.println(F("success!"));

    // print measured data rate
    Serial.print(F("[SX1262] Datarate:\t"));
    Serial.print(radio.getDataRate());
    Serial.println(F(" bps"));
  }
  else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
  {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("too long!"));
  }
  else if (state == RADIOLIB_ERR_TX_TIMEOUT)
  {
    // timeout occured while transmitting packet
    Serial.println(F("timeout!"));
  }
  else
  {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}

void setup()
{
  Serial.begin(115200);
  mySpi.begin();
  Wire.begin(21, 22);
  display.begin(i2c_Address, true);
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();

  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(868.39, 125.0, 12, 7, 0x4A, 14);
  radio.setRfSwitchPins(25, 25);
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
  radio.setRfSwitchPins(25, 25);

#ifdef TEST

  for (int i = 0; i < 40; i++)
  {
    dBase[i].message = String(random(10000));
    dBase[i].rssi = random(100);
    dBase[i].snr = random(100);
  }

#endif
}

void loop()
{
  Serial.print(F("[SX1262] Waiting for incoming transmission ... "));

  // you can receive data as an Arduino String
  // NOTE: receive() is a blocking method!
  //       See example ReceiveInterrupt for details
  //       on non-blocking reception method.

  if (handleButton())
  {

    ESP_LOGD("Button", "PRESSED");
  }

  String str;
  int state = radio.receive(str);
  draw_dBase();
  delay(500);
  // you can also receive data as byte array
  /*
    byte byteArr[8];
    int state = radio.receive(byteArr, 8);
  */

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
    Serial.print(F("[SX1262] Data:\t\t"));
    Serial.println(str);
    Serial.print(F("[SX1262] RSSI:\t\t"));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));
    Serial.print(F("[SX1262] SNR:\t\t"));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

    dBase_add(str, radio.getRSSI(), radio.getSNR());
  }
  else if (state == RADIOLIB_ERR_RX_TIMEOUT)
  {
    Serial.println(F("timeout!"));
  }
  else if (state == RADIOLIB_ERR_CRC_MISMATCH)
  {
    Serial.println(F("CRC error!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}
