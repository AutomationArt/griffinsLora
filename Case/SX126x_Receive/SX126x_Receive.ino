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

// include the library
#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define FreeButton 36

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels      

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 

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

int draw_dBase(){


 display.clearDisplay();

  for(int16_t i=0; i<display.height()/2; i+=2) {
    display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn rectangle
    delay(1);
  }



  display.clearDisplay();

  display.setTextSize(1);            
  display.setTextColor(SSD1306_WHITE);  
  display.setCursor(20,20);           
  display.println(F("Hello, world!"));  

  display.display();

return 0;
}

bool handleButton() {
  static bool previousButtonState = LOW;
  bool currentButtonState = digitalRead(FreeButton);
  if (currentButtonState != previousButtonState) {
    previousButtonState = currentButtonState;
    if (currentButtonState == HIGH) {
      return true;
    } else {
      return false;
    }
  }
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

void setup()
{
  Serial.begin(115200);
  mySpi.begin();
  Wire.begin(21,22);        

  if (!display.begin(SSD1306_EXTERNALVCC, SCREEN_ADDRESS))        
  {
    Serial.println(F("SSD1306 allocation failed"));
  } else {
    Serial.println("Error display");  
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin(868.39, 125.0, 12, 7, 0x4A, 14);
  radio.setRfSwitchPins(25, 25);
  //  int state = radio.begin(868.0, 125.0, 8, 5);
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
}

void loop()
{
  Serial.print(F("[SX1262] Waiting for incoming transmission ... "));

  // you can receive data as an Arduino String
  // NOTE: receive() is a blocking method!
  //       See example ReceiveInterrupt for details
  //       on non-blocking reception method.
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
    // packet was successfully received
    Serial.println(F("success!"));

    // print the data of the packet
    Serial.print(F("[SX1262] Data:\t\t"));
    Serial.println(str);

    // print the RSSI (Received Signal Strength Indicator)
    // of the last received packet
    Serial.print(F("[SX1262] RSSI:\t\t"));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    // print the SNR (Signal-to-Noise Ratio)
    // of the last received packet
    Serial.print(F("[SX1262] SNR:\t\t"));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

    dBase_add(str,radio.getRSSI(),radio.getSNR());


  }
  else if (state == RADIOLIB_ERR_RX_TIMEOUT)
  {
    // timeout occurred while waiting for a packet
    Serial.println(F("timeout!"));
  }
  else if (state == RADIOLIB_ERR_CRC_MISMATCH)
  {
    // packet was received, but is malformed
    Serial.println(F("CRC error!"));
  }
  else
  {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}
