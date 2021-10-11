#include "ir_remote.h"
#include "RFID.h"
#include "wifi.h"
#include "servo_motor.h"
#include "ultrasonic.h"
//#include "lcd_i2c.h"
#include "status.h"
#include "RGBLed.h"
#include "MyEEPROM.h"

#define SERIAL_DEBUG

#define LOCK 0
#define UNLOCK 90
#define CDS_PIN A0
#define BOX_DIST 5
#define CDS_LIMIT 50


IRremote irremote;
RFID rfid;
Wifi wifi;
Servo0 serv;
Ultrasonic sonic;
RGBLed led;
MyEEPROM eeprom;
//LcdI2C lcd;

//boolean rfid_valid = false;
//boolean ship_valid = false;
//boolean box_valid = false;
boolean box_closed = false;

int getCds()
{
  return map(analogRead(CDS_PIN), 0, 1023, 0, 100);
}

void setup() {

#ifdef SERIAL_DEBUG
  Serial.begin(115200);
#endif //SERIAL_DEBUG

  rfid.init();
  irremote.init();
  wifi.init();
  serv.init(LOCK, UNLOCK);
  sonic.init();
  led.init(0, 255, 0);

  
  
  //lcd.init();

  box_closed = false;
}

void loop() {
  if (rfid.update())
  {
    if (wifi.reconnect())
    {
      // lcd server down
    }
    else
    {
      rfid.validCheck(&wifi);
    }
//      char id[30] = {0};
//      sprintf (id, "[JHJ_MAN]%s\n", rfid.getCardId());
//      #ifdef SERIAL_DEBUG
//      Serial.print("card id: ");
//      Serial.println(id);
//      #endif // SERIAL_DEBUG
//      wifi.sendMsg(id);
//      rfid.cardReset();
  }

  if (irremote.update(&wifi))
  {
    if (wifi.reconnect())
    {
      // lcd server down
    }

  }

  STATUS status = wifi.update();
  switch (status)
  {
    case NONE: break;
    case RFID_OK:
      #ifdef SERIAL_DEBUG
      Serial.println("RFID_OK");
      #endif
      if (box_closed == true)
      {
        #ifdef SERIAL_DEBUG
        Serial.println("UNLOCK");
        #endif
        box_closed = false;
        // 잠금 해제 lcd unlock print
        led.blink(0, 0, 255, 100);
        wifi.sendMsg("[JHJ_MAN]unlock\n");
//        char temp[30];
//        sprintf (temp, "[CARDDEL]%s\n", eeprom.read(0));
//        eeprom.clear(0);
        
        serv.rotate(UNLOCK);
//        lcd.clear();
//        lcd.print(0, 0, "CARD Valid..");
//        lcd.print(0, 1, "BOX_UNLOCK!");
      }
      break;
    case RFID_FAIL:
      // lcd fail 문구 출력
      #ifdef SERIAL_DEBUG
      Serial.println("RFID_FAIL");
      #endif
      led.blink(255, 0, 0, 100);
      wifi.sendMsg("[JHJ_MAN]card invalid\n");
//      lcd.clear();
//      lcd.print(0, 0, "CARD Invalid..");
      break;

    case SHIP_OK:
      // 초음파, 조도 체크 후 잠금 lcd lock print
      float dist = sonic.getDistance();
      int cds = getCds();
      #ifdef SERIAL_DEBUG
      Serial.println("SHIP_OK");
      Serial.print(dist);
      Serial.println("cm");      
      Serial.print("cds:");
      Serial.println(cds);
      #endif
      if (box_closed == false && dist <= BOX_DIST && getCds() <= CDS_LIMIT)
      {
        #ifdef SERIAL_DEBUG
        Serial.println("LOCK");
        #endif
        box_closed = true;        
        led.blink(0, 0, 255, 100);
        wifi.sendMsg("[JHJ_MAN]The package has arrived!\n");
        serv.rotate(LOCK);
//        lcd.clear();
//        lcd.print(0, 0, "Ship Number OK");
//        lcd.print(0, 1, "BOX LOCK!!");
      }
      else 
      {
        #ifdef SERIAL_DEBUG
        Serial.println("SHIP_FAIL");
        #endif
        led.blink(255, 0, 0, 100);
        wifi.sendMsg("[JHJ_MAN]SHIP@FAIL\n");
//        lcd.clear();
//        lcd.print(0, 0, "box status bad");
      }
      rfid.cardReset();
      break;
    case SHIP_FAIL:
      // 운송장 조회 실패 lcd 출력
      led.blink(255, 0, 0, 100);
      wifi.sendMsg("[JHJ_MAN]SHIP@FAIL\n");
      rfid.cardReset();
//      lcd.clear();
//      lcd.print(0, 0, "invalid info..");
      break;
  }
}
