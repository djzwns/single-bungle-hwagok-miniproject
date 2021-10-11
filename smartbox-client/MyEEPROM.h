#ifndef EEPROM_H
#define EEPROM_H

#include <EEPROM.h>
#define SIZE 15

class MyEEPROM
{
   public:
      void write(char* data, int start);
      char* read(int start);
      void clear(int start);
};

#endif // EEPROM_H
