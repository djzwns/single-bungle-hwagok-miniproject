#ifndef RFID_H
#define RFID_H

#include "wifi.h"
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9
#define SS_PIN 10
#define UID_BYTE rfid.uid.uidByte

class RFID
{
  public: 
    RFID();
    void init();
    boolean update();
    boolean validCheck(Wifi* wifi);
    void cardReset();
    char* getCardId();

  private:
    boolean isCardDetach();
    boolean isSupportCard();
    boolean cardCheck();

    MFRC522 rfid;
    MFRC522::MIFARE_Key key;
    byte nuidPICC[4];
  
};

#endif // RFID_H
