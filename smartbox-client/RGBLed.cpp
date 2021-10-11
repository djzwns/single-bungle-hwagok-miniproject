#include "RGBLed.h"
#include "arduino.h"

void RGBLed::init(byte r, byte g, byte b)
{
   this->r = r;
   this->g = g;
   this->b = b;

   pinMode(LED_R, OUTPUT);
   pinMode(LED_G, OUTPUT);
   pinMode(LED_B, OUTPUT);

   setColor(r, g, b);
}

void RGBLed::setColor(byte r, byte g, byte b)
{
#ifdef PWMPIN
   analogWrite(LED_R, r);
   analogWrite(LED_G, g);
   analogWrite(LED_B, b);   
#endif
#ifndef PWMPIN
   digitalWrite(LED_R, r);
   digitalWrite(LED_G, g);
   digitalWrite(LED_B, b);   
#endif
}

void RGBLed::blink(byte r, byte g, byte b, int ms)
{
   for(byte i = 0; i < 5; ++i)
   {
      setColor(r, g, b);
      delay(ms);
      setColor(0, 0, 0);
      delay(ms);
   }
   setColor(this->r, this->g, this->b);
}
