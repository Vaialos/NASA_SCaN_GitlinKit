#include "Nokia_LCD_functions.h"  //include library to drive NOKIA display
//NOKIA 5110 is, at 5px chars, 12 characters wide
#include <HammingEncDec.h>        // include the Hamming encoder/decoder functionality
#include <OpticalModDemod.h>      // include the modulator/demodulator functionality
OpticalReceiver phototransistor;  // create an instance of the receiver
byte c;         //holds byte returned from receiver
String         parameterValue;      // holds the measurand being built up character-by-character
String         strTemperature, strHumidity; // holds the values of the measurands
String         concatenatedTemperatureString, concatenatedHumidityString;    // holds the values of the measurands
float          temperature_F = 0.0f;    // variable for a Fahrenheit conversion
String         strTemperFahren;         // string for a Fahrenheit conversion
const uint8_t  NOKIA_SCREEN_MAX_CHAR_WIDTH = 12;
const uint8_t  PIN_PHOTOTRANSISTOR = 2;

const unsigned char nasa_worm_BMP [] = {
0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x20, 0x20, 0x70, 0x20, 0x20, 0x00, 0x00,
0x02, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xF0, 0xF8, 0xFC, 0xFE, 0xFE, 0xFE, 0x7E, 0x3C,
0x08, 0xF0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x08, 0x14, 0x08, 0x00, 0x80, 0x60,
0x80, 0x00, 0x00, 0x20, 0x00, 0x82, 0xC5, 0xE2, 0xE0, 0xE0, 0xE0, 0xE0, 0xC0, 0x80, 0x00, 0x00,
0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x01, 0x03, 0x05,
0x05, 0x84, 0xC4, 0xE2, 0xE1, 0xE0, 0xE0, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x04, 0x0A, 0x04,
0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
0xE0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0xC0, 0xC0,
0x81, 0x01, 0x02, 0x0C, 0x02, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0x01, 0x03, 0x07,
0x7F, 0xFF, 0xFF, 0xF8, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x00, 0xC0, 0xF8, 0xFF, 0xFF, 0xFF, 0x1F, 0x03, 0x03, 0x0F, 0x7F, 0xFF, 0xFF, 0xFC, 0xE0,
0x80, 0x00, 0x00, 0x7E, 0xFF, 0xFF, 0xFF, 0xC3, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
0x81, 0x81, 0x81, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0xFE, 0xFF, 0xFF, 0x7F, 0x0F,
0x03, 0x07, 0x3F, 0xFF, 0xFF, 0xFE, 0xF0, 0x80, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x3F, 0xFF, 0xFF, 0xFE, 0xE0, 0x80, 0x80, 0xC0, 0xFF, 0xFF,
0xFF, 0xFF, 0x00, 0x00, 0xF0, 0xFE, 0xFF, 0xFF, 0x1F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x1F, 0x7F, 0xFF, 0xFF, 0xF8, 0xE0, 0x00, 0x81, 0x83, 0x83, 0x87, 0x87, 0x87, 0x87, 0x87,
0x87, 0x87, 0x87, 0x87, 0x87, 0xCF, 0xFF, 0xFF, 0xFE, 0x7C, 0x00, 0xC0, 0xF8, 0xFF, 0xFF, 0xFF,
0x1F, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x3F, 0xFF, 0xFF, 0xFC, 0xE0, 0x80, 0x00,
0x07, 0x07, 0x27, 0x57, 0x50, 0x90, 0x00, 0xE0, 0x10, 0x10, 0xA0, 0x01, 0x83, 0x47, 0x47, 0xC7,
0x07, 0xF7, 0x27, 0x47, 0xF3, 0x01, 0x00, 0x07, 0x07, 0x07, 0x07, 0xF0, 0x00, 0x00, 0x00, 0x00,
0x00, 0xE0, 0x50, 0x50, 0xE0, 0x00, 0x20, 0x53, 0x57, 0x97, 0x07, 0xF7, 0x57, 0x57, 0x17, 0x07,
0xF7, 0x57, 0xD7, 0x27, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xF7, 0x53, 0xD1, 0x20, 0x06, 0xF7,
0x57, 0x57, 0x17, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x50, 0x50, 0xE0, 0x00, 0x37,
0xC7, 0x37, 0x07, 0x06, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
0x00, 0x0D, 0x01, 0x0D, 0x00, 0x01, 0x38, 0x44, 0x55, 0x30, 0x00, 0x44, 0x7C, 0x44, 0x00, 0x05,
0x05, 0x7D, 0x05, 0x04, 0x00, 0x7D, 0x40, 0x40, 0x41, 0x00, 0x45, 0x7D, 0x45, 0x00, 0x00, 0x7D,
0x09, 0x11, 0x7D, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x7C, 0x10, 0x28, 0x44, 0x00, 0x45, 0x7C,
0x44, 0x01, 0x04, 0x05, 0x7D, 0x05, 0x05, 0x00, 0x0D, 0x01, 0x0D, 0x01, 0x00, 0x00, 0x01, 0x00,
0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 
};


void setup() 
{
  Serial.begin(9600);                 // start the serial port on the Arduino
  Serial.println("NASA SCaN Gitlinkit Laser Relay demonstration -- powering on.");
  phototransistor.set_speed(2000);             // laser receive speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  phototransistor.set_rxpin(PIN_PHOTOTRANSISTOR);       // pin the phototransistor is connected to
  phototransistor.set_inverted(true);                       // if receive signal is inverted (Laser on = logic 0) set this to true
  phototransistor.begin();                                  // initialize the receiver

  LCDInit(); //Start the LCD
  /* Cosmetic frivolity on power-up. */
  LCDClear();
  LCDBitmap(nasa_worm_BMP);
  delay(2250);
  LCDClear();
}

/* --------- INTERRUPT SERVICE ROUTINE (ISR) ----------
   Set up an interrupt service routine to receive characters
   Arduino Timer2 reads the LIGHT_RECEIVE_PIN at laser
   receive speed to receive each half bit */
ISR(TIMER2_COMPA_vect)
{
  phototransistor.receive();   // check for a received character every timer interrupt
}

void loop() 
{
  bool received = phototransistor.GetByte(c);     // get a character from the laser receiver if one is available
  if (received)
  {
   // if a character is ready, look at it
   Serial.println(c);
   switch (c)
    {       
      // if the character is a terminator, store what was built in a variable and display it
      case 0:
        char screen_line[NOKIA_SCREEN_MAX_CHAR_WIDTH]; //array holding line we will display to the nokia screen
        memset(screen_line, ' ',NOKIA_SCREEN_MAX_CHAR_WIDTH); //As part of our manual NewLine hack, build a blank 12-char array of spaces
        concatenatedTemperatureString = String("Tmp: ") + strTemperature + String(" C");
        for (int i = 0; (i < concatenatedTemperatureString.length()) && (i < NOKIA_SCREEN_MAX_CHAR_WIDTH); i++)
        {
          screen_line[i] = concatenatedTemperatureString[i];
        }
        LCDString(screen_line);
        
        memset(screen_line, ' ',NOKIA_SCREEN_MAX_CHAR_WIDTH); //As part of our manual NewLine hack, build a blank 12-char array of spaces
        concatenatedHumidityString = String("Hmd: ") + strHumidity + String("%");
        for (int i = 0; (i < concatenatedHumidityString.length()) && (i < NOKIA_SCREEN_MAX_CHAR_WIDTH); i++)
        {
          screen_line[i] = concatenatedHumidityString[i];
        }
        LCDString(screen_line);
        break;       
      case 84:         // ASCII T termination character for temperature, use string built to this point for temp
        strTemperature=parameterValue;
        parameterValue="";
        Serial.print("*= Temp: "); Serial.print(strTemperature); Serial.println(" C =*");
        break; 
      case 72:        // ASCII H termination character for humidity, use string built to this point for humidity
        strHumidity=parameterValue;
        parameterValue="";
        Serial.print("*- Humd: "); Serial.print(strHumidity); Serial.println("%  -*");
        break;
      default :
        parameterValue+=(char)c;  // keep building a string character-by-character until a terminator is found
        break;
    }
  }
} // end main loop
