#ifndef RGB_LED_H
#define RGB_LED_H

#define PWMPIN
#define LED_R A1
#define LED_G A2
#define LED_B A3
#include "arduino.h"

class RGBLed
{
   public:
      void init(byte r, byte g, byte b);
      void setColor(byte r, byte g, byte b);
      void blink(byte r, byte g, byte b, int ms);

   private:
      byte r;
      byte g;
      byte b;
};


#endif // RGB_LED_H
