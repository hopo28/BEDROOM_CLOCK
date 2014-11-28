#include "FastLED.h"
#include "glcdfont.c"
#include <Time.h>  
#include <TimeAlarms.h>
#include <Wire.h>  
#include <DS1307RTC.h>
#include <EEPROM.h>

/* LED LAYOUT FOR NEOPIXEL
  if layout different, need to change setPixel and getPixel
00 08 16
01 09 17
02 10 18
03 11 19
04 12 20
05 13 .
06 14 .
07 15 .
*/

#define LED_PIN 2
#define MATRIX_X 8*3
#define MATRIX_Y 8
#define NUM_LEDS MATRIX_X*MATRIX_Y
#define LED_RUN_LENGTH 8//number of leds per column
#define LED_BRIGHTNESS 10 //default brightness, was 16
#define ALARM_BRIGHTNESS 255

#define LDR_GND A0//fake gnd for LDR
#define LDR_PIN A1//random pin for now
#define LDR_VCC A2//fake vcc for LDR
//#define RAND_PIN

// - - - PLASMA - - -

typedef struct
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
} ColorRGB;
typedef struct 
{
  unsigned char h;
  unsigned char s;
  unsigned char v;
} ColorHSV;
unsigned char plasma[MATRIX_X][MATRIX_Y];
long paletteShift;
float plasmaZoom = 0.75;//0.5 looks better on large display

//versions for serial.print
int alarmHour = 0;
int alarmMin = 0;
int alarmSec = 0;

boolean plasmaMode = false;

boolean autoBright = true;
byte LEDBrightness = LED_BRIGHTNESS;
CRGB leds[NUM_LEDS];

void setup()
{
  // - - - SERIAL - - -
  Serial.begin(9600);
  // - - - RTC - - -
  setSyncProvider(RTC.get);   // the function to get the time from the RTC, gets time now
  setSyncInterval(300);  //default is 300 secs (5 mins), set to 1 hour
  if(timeStatus()!= timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");      
  // - - - LEDS - - -
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(LED_BRIGHTNESS);
  // - - - PLASMA - - -
  
  paletteShift=128000;
  unsigned char bcolor;
  //generate the plasma once - prime it
  for(unsigned char y = 0; y < MATRIX_Y; y++)
    for(unsigned char x = 0; x < MATRIX_X; x++)
    {
      //the plasma buffer is a sum of sines
      bcolor = (unsigned char)
      (
            128.0 + (128.0 * sin(x*8.0 / 16.0))
          + 128.0 + (128.0 * sin(y*8.0 / 16.0))
      ) / 2;
      plasma[x][y] = bcolor;
    }
    
  // - - - LDR - - -
  pinMode(LDR_GND, OUTPUT);
  digitalWrite(LDR_GND, LOW);
  pinMode(LDR_PIN, INPUT);
  pinMode(LDR_VCC, OUTPUT);
  digitalWrite(LDR_VCC, HIGH);
  // - - - RAND - - -
  //pinMode(RAND_PIN, INPUT);
  //randomSeed(analogRead(RAND_PIN));
  FastLED.showColor(CRGB::Black);//start black
  retriveAlarm();
  printMenu();
  startUp();
}

void loop()
{
  if (Serial.available() > 0)
  {
    doMenu(Serial.read());
  }

  if(!plasmaMode)
    displayClock();  
  else
    plasma_morph();   

  checkLight();//no LDR hooked up atm
  
  if( hour() == alarmHour &&
      minute() == alarmMin &&
      second() == alarmSec)
  {  
    TheAlarm();
  }
  //checkButtons();
  //checkChime();//on the hour
 
    
  /*print numbers 0-9
  for(int i = 0; i < 10; i++)
  {
   FastLED.clear();
    setChar(0,0,'0'+i,CRGB::White);
    FastLED.show();    
    delay(500);
  }
  */
  
  /*create rainbow diagonal lines
  for(int i = 0; i < 8; i++)
  {
    setPixel(i,i,Wheel(i*(255/8)));
    setPixel(7-i,i,Wheel(i*(255/8)));
  }
  FastLED.show();  
  */
  
  //test each led in sequence
  //strandTest(CRGB::White,100);
  
  /*rainbow all leds
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = Wheel(i*(255/NUM_LEDS));
  }
  FastLED.show();
  */
}

void displayClock()
{  
//  FastLED.showColor(CRGB::Black);//need to fill black but dont show it
  for(int i = 0; i < NUM_LEDS; i++)//clear previous array data
  {
      leds[i] = CRGB::Black;
  }

  char char0,char1,char2,char3;
  int mins = minute();
  int hr = hour();
  int sec = second();
  CRGB secColour, minColour, hrColour;
  
  if(hr<10)
  {
    char0 = '0';//preceding 0
    char1 = hr + '0';
  }
  else
  {
    char0 = ((hr - (hr % 10))/10) + '0';
    char1 = (hr % 10) + '0';
  }
  
  //do mins
  if(mins<10)
  {
    char2 = '0';//preceding 0
    char3 = mins + '0';
  }
  else
  {
    char2 = ((mins - (mins % 10)) /10) + '0';
    char3 = (mins % 10) + '0';
  }

  if(LEDBrightness <= 2)//ultradim mode
  {
    secColour = CRGB::Red;
    minColour = CRGB::Red;
    hrColour = CRGB::Red;
    if(LEDS.getBrightness() == 2)//flipit causing 1/3rd dimmest brightness
      LEDS.setBrightness(0);
    else if(LEDS.getBrightness() == 0)
      LEDS.setBrightness(1);
    else//brightness must be 1
      LEDS.setBrightness(2);
  }
  else if(LEDBrightness <= 5)//dim mode
  {
    secColour = CRGB::Red;
    minColour = CRGB::Red;
    hrColour = CRGB::Red;
    LEDS.setBrightness(2);//rgb (dimmest standard mode)
  }
  else//normal
  {
    secColour = Wheel(sec*(255/60));
    minColour = Wheel(mins*(255/60));
    hrColour = Wheel(hr*(255/24));
    LEDS.setBrightness(LEDBrightness);
  }
  
  float secTick = ((float)sec/(float)MATRIX_X);

  setPixel(secTick, 7, secColour);//tick every 2.5 seconds

  //using THIN NUMBERS(its the only way it will fit)
  setChar(1,0, char0, hrColour);//Hour tens
  setChar(6,0, char1, hrColour);//Hour units
  setChar(10,0, ':', secColour);//colon
  setChar(14,0, char2, minColour);//Min tens
  setChar(19,0, char3, minColour);//Min units
  
  FastLED.show();
}


void doMenu(char selection)
{
  switch(selection)
  {
    case 'G'://print time out
	  digitalClockDisplay();
	  digitalAlarmDisplay();
          Serial.print(" Brightness: ");
          Serial.println(LEDBrightness);
	  break;
    case 'S'://set time menu
	  setTimeMenu();
          printMenu();
	  break;
    case 'A'://set alarm menu
	  setAlarmMenu();
          printMenu();
	  break;
    case 'B'://set brightness menu
	  setBrightnessMenu();
          printMenu();
	  break;
    case 'P'://toggle plasmaDemo
          plasmaMode = !plasmaMode;
          printMenu();
	  break;
    default:
	  Serial.println("Unknown command");
	  break;
  }
}

void checkChime()
{
  if (minute() == 0 && second() == 0)
  {
    now();//gets time now, forces resync
    setSyncInterval(600);//set sync time to every 10 mins to make sure
    //DO CHIME HERE
  }
}

void TheAlarm()
{
  Serial.println("ALARM!");
  LEDS.setBrightness(ALARM_BRIGHTNESS);//full bright
  for(int i = 0; i < 10; i++)
  {
    FastLED.showColor(CRGB::White);
    delay(100);
    FastLED.showColor(CRGB::Black);
    delay(100);
  }
  LEDS.setBrightness(LED_BRIGHTNESS);//reset brightness
}

void checkLight()
{
  if(autoBright)
  {
  //  Serial.println(analogRead(LDR_PIN));
    int reading = 0;
    reading += analogRead(LDR_PIN);
    reading += analogRead(LDR_PIN);
    reading += analogRead(LDR_PIN);
    reading += analogRead(LDR_PIN);
    reading += analogRead(LDR_PIN);
    reading = reading / 5;//average it         
    int brightness = map(reading,0,1024,0,255);
    LEDBrightness = brightness;//set the var, not the display, let the clock function handle that
  //  LEDS.setBrightness(brightness);
  }
}
