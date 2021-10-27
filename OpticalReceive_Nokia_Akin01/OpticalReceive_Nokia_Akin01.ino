


#include "Nokia_LCD_functions.h"    //include library to drive NOKIA display. TODO figure out how to structure this library so it doesn't need to be copied locally.
#include <HammingEncDec.h>          // include the Hamming encoder/decoder functionality
#include <OpticalModDemod.h>        // include the modulator/demodulator functionality
#include <string.h>
OpticalReceiver phototransistor;    // create an instance of the receiver
byte           c;                   //holds byte returned from receiver
String         parameterValue;      // holds the measurand being built up character-by-character
String         strTemperature, strHumidity; // holds the values of the measurands
String         concatenatedTemperatureString, concatenatedHumidityString;    // holds the values of the measurands
float          temperature_F = 0.0f;    // variable for a Fahrenheit conversion TODO
String         strTemperFahren;     // string for a Fahrenheit conversion TODO
const uint8_t  NOKIA_SCREEN_MAX_CHAR_WIDTH = 12;
const int      LASER_TRANSMIT_SPEED = 2000; //Don't edit this unless you really know what you're doing. Keeps TX and RX in phase.
const uint8_t  PIN_PHOTOTRANSISTOR = 2; //Pin on the Arduino for the phototransistor (receiver 'eye')

int            link_timeout = 1250; // if no valid characters received in this time period, assume the link is bad
unsigned long  time_since_last_character_received = 0; //helps decide when to message about a bad link
unsigned long  time_now = 0;        //also helps to decide about bad link
bool           is_the_link_good = true; //boolean for the same bad link decision
const long int ANTISPAM_COUNTER_MAX = 16000;
int            antispam_counter = 0;
uint8_t        ellipsis_iterator = 0; //iterator to animate a ".  " ".. " "..."
uint8_t        j = 0; //iterator to periodically clear the screen to avoid too much pile-up of junk

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
  Serial.println("NASA SCaN Gitlinkit Laser Relay demonstration -- powering on RECEIVER.");
  phototransistor.set_speed(LASER_TRANSMIT_SPEED);          // laser receive speed - should be 500+ bits/second, nominal 2000 (=2KHz)
  phototransistor.set_rxpin(PIN_PHOTOTRANSISTOR);           // pin the phototransistor is connected to
  phototransistor.set_inverted(true);                       // if receive signal is inverted (Laser on = logic 0) set this to true
  phototransistor.begin();                                  // initialize the receiver

  LCDInit(); //Start the LCD
  /* Cosmetic frivolity on power-up. */
  LCDClear();
  LCDBitmap(nasa_worm_BMP);
  delay(2750);
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
    if (j < 101) //arbitrary number 
    {
      j++;
    }
    else
    {
      j = 0;
      LCDClear(); //Periodically reset the screen every _n_ receives, just for cleanliness.
    }
    time_since_last_character_received = millis(); //pulls the time elapsed from the Arduino startup in and stores it
    switch (c)
    {
      // if the character is a terminator, store what was built in a variable and display it
      case 0:
        //This means a full packet is complete, so now we can push a screen update. Do this sparingly, as it can slow down the program
        //and goober the data stream; things have relatively tight processor tolerances right now.
        //LCDClear();
        
        //Builds an array of a full line (12 blank chars by default), then overwrites our intended string char by char until it's done or full, then displays.
        //Will truncate anything passed to it that's more than NOKIA_SCREEN_MAX_CHAR_WIDTH.
        //This is a hack to support the otherwise lack of newline for the Nokia. Credit to DanK "Memes" Koris.
        // Tried to make this into a function but it was picking up random chunks of garbage so... we're back here.

        char screen_line[NOKIA_SCREEN_MAX_CHAR_WIDTH]; //array holding line we will display to the nokia screen
        //---------------------------------------------------
        for (int i = 0; (i < NOKIA_SCREEN_MAX_CHAR_WIDTH); i++)
        {
          screen_line[i] = ' ';
        }
        Serial.println(screen_line);
        concatenatedTemperatureString = String("T: ") + strTemperature + String("     C");
        Serial.print("temp string length is ");Serial.println(concatenatedTemperatureString.length());
        for (int i = 0; (i < concatenatedTemperatureString.length()) && (i < NOKIA_SCREEN_MAX_CHAR_WIDTH); i++)
        {
          screen_line[i] = concatenatedTemperatureString[i];
        }
        Serial.println(screen_line);
        LCDString(screen_line);
        //---------------------------------------------------
        memset(screen_line, ' ',NOKIA_SCREEN_MAX_CHAR_WIDTH); 
        concatenatedHumidityString = String("Hmd: ") + strHumidity + String("%");
        for (int i = 0; (i < concatenatedHumidityString.length()) && (i < NOKIA_SCREEN_MAX_CHAR_WIDTH); i++)
        {
          screen_line[i] = concatenatedHumidityString[i];
        }
        LCDString(screen_line);
        //---------------------------------------------------
        break;
      case 84:         // ASCII T termination character for temperature, use string built to this point for temp
        strTemperature = parameterValue;
        parameterValue = "";
        Serial.print("*= TEMP:  ");Serial.print(strTemperature);Serial.println(" C =*");
        break;
      case 72:        // ASCII H termination character for humidity, use string built to this point for humidity
        strHumidity = parameterValue;
        parameterValue = "";
        Serial.print("*- HUMID: ");Serial.print(strHumidity);Serial.println("% -*");
        break;
      default :
        parameterValue += (char)c; // keep building a string character-by-character until a terminator is found
        break;
    }
  }
  else
  {
    is_the_link_good = (millis() <= (time_since_last_character_received + link_timeout)); //Check if we're within the timeout parameter
    if (!(is_the_link_good))
    {
      if (antispam_counter >= ANTISPAM_COUNTER_MAX)
      {
        Serial.print(antispam_counter); Serial.print("\t"); Serial.println("Waiting for laser.");
        switch (ellipsis_iterator)
        {
          case 0:
            LCDString("No laser    ");
            break;
          case 1:
            LCDString("No laser.   ");
            break;
          case 2:
            LCDString("No laser..  ");
            break;
          case 3:
            LCDString("No laser... ");
            delay(25);
            break;
          case 4:
            LCDString("No laser....");
            delay(25);
            LCDClear();  
            break;        
          default:
            ellipsis_iterator = 0;
            break;
        }
        ellipsis_iterator++;
        antispam_counter = 0;
      }
      else
      {
        antispam_counter++;
      }
    }
  }
} // end main loop
