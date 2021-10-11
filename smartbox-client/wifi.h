#ifndef WIFI_H
#define WIFI_H

#define AP_SSID "iotsystem1"
#define AP_PASS "iotsystem10"
#define SERVER_NAME "192.168.10.88"
#define SERVER_PORT 5000  
#define LOGID "JHJ_ARD"
#define PASSWD "PASSWD"

#define WIFITX 7  //7:TX -->ESP8266 RX
#define WIFIRX 6 //6:RX-->ESP8266 TX

#define CMD_SIZE 50
#define ARR_CNT 5 

#include "status.h"
#include "WiFiEsp.h"
#include "SoftwareSerial.h"

#include <Wire.h>

#define LED_PIN 5

class Wifi
{
  public: 
    Wifi() : wifiSerial(WIFIRX, WIFITX) {}
    void init();
    STATUS update();
    boolean reconnect();
    void sendMsg(char* msg);

  
  private:
    char sendBuf[CMD_SIZE];    
    bool timerIsrFlag = false;
    unsigned int secCount;
    char sendId[10]="JHJ_LDP";
    SoftwareSerial wifiSerial; 
    WiFiEspClient client;

    STATUS socketEvent();    
    void wifi_Setup();   
    void wifi_Init(); 
    int server_Connect();
    void printWifiStatus();
};
#endif // WIFI_H
