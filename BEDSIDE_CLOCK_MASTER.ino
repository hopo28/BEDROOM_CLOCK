/*
Colorduino master controller
generates random colours and sends it
*/

#include <Wire.h>

#define X_SIZE 8
#define Y_SIZE 8

void setup()
{
  Wire.begin(); // join i2c bus (address optional for master)
  randomSeed(analogRead(7));
}

void loop()
{
  Wire.beginTransmission(4); // transmit to device #4
  for(int x = 0; x < X_SIZE; x++)
  {
    for(int y = 0; y < Y_SIZE; y++)
    {
      Wire.write(random(0,255));//R
      Wire.write(random(0,255));//G
      Wire.write(random(0,255));//B
    }
  }
  Wire.endTransmission();    // stop transmitting
  delay(1000);
}


