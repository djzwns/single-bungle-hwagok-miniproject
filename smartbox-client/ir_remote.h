#ifndef IR_REMOTE_H
#define IR_REMOTE_H

#include "wifi.h"
#include "MyEEPROM.h"
#include <IRremote.h>
#include <stdio.h>
#include <string.h>

#define IRPIN 8
#define NUM_0   0xFF6897
#define NUM_1   0xFF30CF
#define NUM_2   0xFF18E7
#define NUM_3   0xFF7A85
#define NUM_4   0xFF10EF
#define NUM_5   0xFF38C7
#define NUM_6   0xFF5AA5
#define NUM_7   0xFF42BD
#define NUM_8   0xFF4AB5
#define NUM_9   0xFF52AD
#define BTN_OK  0xFF9867 // 100+ button
#define BTN_NO  0xFFB04F // 200+ button
#define BTN_BS  0xFFE01F // - button
#define LED_PIN 5

class IRremote
{  
  public:
    IRremote() : irrecv(IRPIN) { }
    void init()
    {
      irrecv.enableIRIn();
      pinMode (LED_PIN, OUTPUT);
    }
    
    boolean update(Wifi* wifi)
    {
      if (irrecv.decode(&results))
      {
        Serial.println(results.value, HEX);
    
//        if (results.value == NUM_0)
//          digitalWrite(LED_PIN, HIGH);
//        else if (results.value == NUM_1)
//          digitalWrite(LED_PIN, LOW);

        if (results.value == BTN_OK)
        {
          char msg[50];
          sprintf (msg, "[SPVALID]%s\n", shippingNumber);
          Serial.println(msg);
          wifi->sendMsg(msg);
          eeprom.write(shippingNumber, 0);
          memset(shippingNumber, 0, sizeof(shippingNumber));
        }
        else if (results.value == BTN_NO)
        {
          memset(shippingNumber, 0, sizeof(shippingNumber));
        }
        else if (results.value == BTN_BS)
        {
          // 한 글자씩 지울 예정
        }
        else if(isNum(results.value))
        {
          char temp[2];
          sprintf (temp, "%d", btnToNum(results.value));
          strcat(shippingNumber, temp);
          Serial.println(shippingNumber);
        }
        delay(30);
        irrecv.resume();

        return true;
      }

      return false;
    }
    
  private:
    IRrecv irrecv;
    decode_results results;
    MyEEPROM eeprom;

    char shippingNumber[15];

    boolean isNum(unsigned long btn)
    {
      return btn == NUM_0 || btn == NUM_1 || btn == NUM_2 || btn == NUM_3 || btn == NUM_4 || btn == NUM_5 || btn == NUM_6 || btn == NUM_7 || btn == NUM_8 || btn == NUM_9; 
    }

    int btnToNum(unsigned long btn)
    {
      switch(btn)
      {
        case NUM_0: return 0;
        case NUM_1: return 1;
        case NUM_2: return 2;
        case NUM_3: return 3;
        case NUM_4: return 4;
        case NUM_5: return 5;
        case NUM_6: return 6;
        case NUM_7: return 7;
        case NUM_8: return 8;
        case NUM_9: return 9;
        default: return -1;
      }
    }
};

#endif // IR_REMOTE_H
