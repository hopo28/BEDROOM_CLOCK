
/*------------------------------------------- GRAPHIC THINGS -------------------------------------------*/

void startUp()
{//do a startup sequence (quick)
  LEDS.setBrightness(64);//reasonable for starters

  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = Wheel(i*(255/NUM_LEDS));
  }
  FastLED.show();
  delay(500);
  strandTest(CRGB::White,10);
  delay(500);
}

void setPixel(int x, int y, CRGB colour)
{
  int i = (x*LED_RUN_LENGTH)+(LED_RUN_LENGTH-y)-1;
  leds[i] = colour;
}

void strandTest(CRGB Color,int Delay)
{
   for(int i = 0; i < NUM_LEDS; i = i + 1)
   {
      leds[i] = Color;
      FastLED.show();
      delay(Delay);
      leds[i] = CRGB::Black;
   }
}

CRGB Wheel(byte WheelPos)
{
	if(WheelPos < 85)//in first 3rd
	{
		return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);//no blue component
	}
	else if(WheelPos < 170) //in second 3rd
	{
		WheelPos -= 85;
		return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);//no green component
	}
	else//in last 3rd
	{
		WheelPos -= 170;
		return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);//no red component
	}
}

// Draw a character
void setChar(int16_t x, int16_t y, unsigned char c, CRGB color)
{
  for (int8_t i=0; i<6; i++ )
  {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++)
    {
      if (line & 0x1)
      {
        setPixel(x+i, y+j, color);
      }
      line >>= 1;
    }
  }
}

/*------------------------------------------- PRINTY THINGS -------------------------------------------*/

void printMenu()
{
  Serial.println(" -- Hopo's LED Clock -- ");
  digitalClockDisplay();
  digitalAlarmDisplay();
  Serial.print(" Brightness: ");
  Serial.println(LEDBrightness);
  Serial.println(" - MENU - ");
  Serial.println(" G = Get Settings");
  Serial.println(" S = Set Date/Time");
  Serial.println(" A = Set Alarm");
  Serial.println(" B = Set Brightness");
  Serial.println(" P = Toggle Plasma");
}

void setBrightnessMenu()
{
  Serial.println("Enter Brightness (000-255) [000=Auto]");
  byte lux = get3DigitFromSerial();
  if(lux == 000)
  {
    autoBright = true;
  }
  else
  {
    autoBright = false;
    LEDBrightness = lux;
  }
  //LEDS.setBrightness(LEDBrightness);//let the display function handle that
//  saveBrightness();
}

void setTimeMenu()
{
  int timeDatArray[6] = {0,0,0,0,0,0};
 
  Serial.println("Enter Year (YY)");
  timeDatArray[5] = get2DigitFromSerial();
  Serial.println("Enter Month (MM)");
  timeDatArray[4] = get2DigitFromSerial();
  Serial.println("Enter Day (DD)");
  timeDatArray[3] = get2DigitFromSerial();
  Serial.println("Enter Hour (hh)");
  timeDatArray[0] = get2DigitFromSerial();
  Serial.println("Enter Minute (mm)");
  timeDatArray[1] = get2DigitFromSerial();
  Serial.println("Enter Second (ss)");
  timeDatArray[2] = get2DigitFromSerial();

  setTime(timeDatArray[0],
          timeDatArray[1],
          timeDatArray[2],
          timeDatArray[3],
          timeDatArray[4],
          timeDatArray[5]);

  time_t t = now();
  RTC.set(t);  
}

void setAlarmMenu()
{
  Serial.println("Enter Alarm Hour (hh)");
  alarmHour = get2DigitFromSerial();
  Serial.println("Enter Alarm Minute (mm)");
  alarmMin = get2DigitFromSerial();
  Serial.println("Enter Alarm Second (ss)");
  alarmSec = get2DigitFromSerial();
  saveAlarm();
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(" Time:  ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println(); 
}

void digitalAlarmDisplay()
{
  Serial.print(" Alarm: ");
  Serial.print(alarmHour);
  printDigits(alarmMin);
  printDigits(alarmSec);
  Serial.println(" Daily");
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

/*------------------------------------------- INPUT THINGS -------------------------------------------*/

int get2DigitFromSerial()
{
  int retVal = 0;
  while (!(Serial.available() > 0));
  {
    retVal += (10 * (Serial.read() - '0'));
  }
  while (!(Serial.available() > 0));
  {
    retVal += (Serial.read() - '0');
  }
  return retVal;
}

int get3DigitFromSerial()
{
  int retVal = 0;
  while (!(Serial.available() > 0));
  {
    retVal += (100 * (Serial.read() - '0'));
  }
  while (!(Serial.available() > 0));
  {
    retVal += (10 * (Serial.read() - '0'));
  }
  while (!(Serial.available() > 0));
  {
    retVal += (Serial.read() - '0');
  }
  return retVal;
}

/*------------------------------------------- PLASMA DEMO STUFF -------------------------------------------*/

//Converts an HSV color to RGB color
void HSVtoRGB(void *vRGB, void *vHSV) 
{
  float r, g, b, h, s, v; //this function works with floats between 0 and 1
  float f, p, q, t;
  int i;
  ColorRGB *colorRGB=(ColorRGB *)vRGB;
  ColorHSV *colorHSV=(ColorHSV *)vHSV;

  h = (float)(colorHSV->h / 256.0);
  s = (float)(colorHSV->s / 256.0);
  v = (float)(colorHSV->v / 256.0);

  //if saturation is 0, the color is a shade of grey
  if(s == 0.0) {
    b = v;
    g = b;
    r = g;
  }
  //if saturation > 0, more complex calculations are needed
  else
  {
    h *= 6.0; //to bring hue to a number between 0 and 6, better for the calculations
    i = (int)(floor(h)); //e.g. 2.7 becomes 2 and 3.01 becomes 3 or 4.9999 becomes 4
    f = h - i;//the fractional part of h

    p = (float)(v * (1.0 - s));
    q = (float)(v * (1.0 - (s * f)));
    t = (float)(v * (1.0 - (s * (1.0 - f))));

    switch(i)
    {
      case 0: r=v; g=t; b=p; break;
      case 1: r=q; g=v; b=p; break;
      case 2: r=p; g=v; b=t; break;
      case 3: r=p; g=q; b=v; break;
      case 4: r=t; g=p; b=v; break;
      case 5: r=v; g=p; b=q; break;
      default: r = g = b = 0; break;
    }
  }
  colorRGB->r = (int)(r * 255.0);
  colorRGB->g = (int)(g * 255.0);
  colorRGB->b = (int)(b * 255.0);
}

float dist(float a, float b, float c, float d) 
{
  return (sqrt((c-a)*(c-a)+(d-b)*(d-b)) * plasmaZoom);
}

void plasma_morph()
{
  LEDS.setBrightness(LEDBrightness);
  unsigned char x,y;
  float value;
  ColorRGB colorRGB;
  ColorHSV colorHSV;

  for(y = 0; y < MATRIX_Y; y++)
  {
    for(x = 0; x < MATRIX_X; x++)
    {
	value = sin(dist(x + paletteShift, y, 128.0, 128.0) / 8.0)
	  + sin(dist(x, y, 64.0, 64.0) / 8.0)
	  + sin(dist(x, y + paletteShift / 7, 192.0, 64) / 7.0)
	  + sin(dist(x, y, 192.0, 100.0) / 8.0);
	colorHSV.h=(unsigned char)((value) * 128)&0xff;
	colorHSV.s=255; 
	colorHSV.v=255;
	HSVtoRGB(&colorRGB, &colorHSV);
	
	setPixel(x, y, CRGB(colorRGB.r, colorRGB.g, colorRGB.b));
    }
  }
  paletteShift++;

FastLED.show();
}

/*------------------------------------------- EEPROM THINGS -------------------------------------------*/

void saveAlarm()
{
  EEPROM.write(0, alarmHour);
  EEPROM.write(1, alarmMin);
  EEPROM.write(2, alarmSec);
  Alarm.alarmRepeat(alarmHour,alarmMin,alarmSec, TheAlarm);
}

void retriveAlarm()
{
  alarmHour = EEPROM.read(0);
  alarmMin = EEPROM.read(1);
  alarmSec = EEPROM.read(2);
  Alarm.alarmRepeat(alarmHour,alarmMin,alarmSec, TheAlarm);
}

void saveBrightness()
{
  EEPROM.write(3, LEDBrightness);
}

void retriveBrightness()
{
  LEDBrightness = EEPROM.read(3);
}
