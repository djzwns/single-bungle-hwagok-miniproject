#include "RFID.h"
#include "arduino.h"
#include <stdio.h>

#define SERIAL_DEBUG


RFID::RFID() : rfid(SS_PIN, RST_PIN)
{
  
}

void RFID::init()
{
  SPI.begin();
  rfid.PCD_Init();
  
  for (byte i = 0; i < 6; ++i)
    key.keyByte[i] = 0xFF;

#ifdef SERIAL_DEBUG
  Serial.println(F("This code scan the MIFARE Classic NUID."));
  Serial.print(F("Using the following key:"));
#endif //SERIAL_DEBUG
}

void RFID::cardReset()
{
  for (byte i = 0; i < 4; ++i)
    nuidPICC[i] = 0xFF;
}

char* RFID::getCardId()
{
  char id[20];
  sprintf(id, "%d.%d.%d.%d", nuidPICC[0], nuidPICC[1], nuidPICC[2], nuidPICC[3]);
  return id;
}

boolean RFID::isCardDetach()
{
  // 카드 인식 확인 안되면 리턴
  if ( !rfid.PICC_IsNewCardPresent() )
    return false;

  // id를 읽지 못했으면 리턴
  if ( !rfid.PICC_ReadCardSerial() )
    return false;

   return true;
}

boolean RFID::isSupportCard()
{
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

#ifdef SERIAL_DEBUG
  Serial.print(F("PICC type: "));
  Serial.println(rfid.PICC_GetTypeName(piccType));
#endif //SERIAL_DEBUG

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {

#ifdef SERIAL_DEBUG
    Serial.println(F("Your tag is not of type MIFARE Classic."));
#endif //SERIAL_DEBUG
    return false;
  }

  return true;
}

boolean RFID::cardCheck()
{  
  if (UID_BYTE[0] != nuidPICC[0] || UID_BYTE[1] != nuidPICC[1] ||
      UID_BYTE[2] != nuidPICC[2] || UID_BYTE[3] != nuidPICC[3] )
  {
#ifdef SERIAL_DEBUG
    Serial.println (F("A new card has been detected."));
#endif //SERIAL_DEBUG

    // nuid save
    for (byte i = 0; i < 4; ++i)
      nuidPICC[i] = UID_BYTE[i];

#ifdef SERIAL_DEBUG
    Serial.print(F("NUID tag: "));
    for (byte i = 0; i < 4; ++i) {
      Serial.print(UID_BYTE[i]);
      Serial.print(F(", "));
    }
    Serial.println(rfid.uid.size);
#endif //SERIAL_DEBUG

    return true;
  }

  return false;
}

boolean RFID::validCheck(Wifi* wifi)
{
  // 서버에 nuid 전송 후 결과 값 받는 코드
  // 확인 된 카드의 nuid를 서버로 전송
  // 서버에서 rfid 데이터베이스 확인
  // 등록된 카드라면 true 아니면 false

  char buf[30] = {0};
  sprintf (buf, "[RFVALID]%d.%d.%d.%d\n", nuidPICC[0], nuidPICC[1], nuidPICC[2], nuidPICC[3]);
  wifi->sendMsg(buf);
  return false;
}

boolean RFID::update()
{
  if ( !isCardDetach() )
    return false;

  if ( !isSupportCard() )
    return false;
    
  if ( !cardCheck() )
    return false;

  return true;
}
