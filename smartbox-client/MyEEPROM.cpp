#include "MyEEPROM.h"
#include "arduino.h"
#include <string.h>
//#include <EEPROM.h>
//EEPROM.write(주소, 저장할값);
//EEPROM.read(주소);
void MyEEPROM::write(char* data, int start)
{
   unsigned char addr, len = start + strlen(data);
   for (addr = start; addr < len; ++addr)
   {
      EEPROM.write(addr, data[addr]);
      delay(100);
   }
}

char* MyEEPROM::read(int start)
{
   byte addr = start;
   char data[SIZE] = {0};
   do 
   {
      unsigned char temp;
      temp = EEPROM.read(addr++);
      if (strcmp("0", temp) == 0)
         break;   
      else strcat(data, temp);
      delay(100);
   } while(addr < start + SIZE);
   strcat(data, "\0");

   return data;
}

void MyEEPROM::clear(int start)
{
   for (int i = start; i < start + SIZE; ++i) 
      EEPROM.write(i, 0);
}
