#include "wifi.h"
#include "arduino.h"

#define DEBUG
//#define DEBUG_WIFI
void Wifi::init()
{
  wifi_Setup();
}

STATUS Wifi::update() 
{
  STATUS status = NONE;
    if(client.available()) {
      status = socketEvent();
    }
    return status;
}

boolean Wifi::reconnect()
{
  if (!client.connected())
  {
    server_Connect();
    return true;
  }
  return false;
}

void Wifi::sendMsg(char* msg)
{
  client.write(msg,strlen(msg));
  client.flush();
}

STATUS Wifi::socketEvent()
{
  int i=0;
  char * pToken;
  char * pArray[ARR_CNT]={0};
  char recvBuf[CMD_SIZE]={0}; 
  int len;
  STATUS status = NONE;

  sendBuf[0] ='\0';
  len = client.readBytesUntil('\n',recvBuf,CMD_SIZE);
  client.flush();
 
#ifdef DEBUG
  Serial.print("recv : ");
  Serial.print(recvBuf);
#endif
  pToken = strtok(recvBuf,"[@]");
  while(pToken != NULL)
  {
    pArray[i] =  pToken;
    if(++i >= ARR_CNT)
      break;
    pToken = strtok(NULL,"[@]");
  }
  
  if(!strncmp(pArray[1]," New",4))  // New Connected
  {
    Serial.write('\n');
    return ;
  }
  else if(!strncmp(pArray[1]," Alr",4)) //Already logged
  {
    Serial.write('\n');
    client.stop();
    server_Connect();
    return ;
  }
  else if (!strcmp(pArray[1], "CARD"))
  {
    if (!strcmp(pArray[2], "OK"))
    {
      // 카드 검증 완료
      status = RFID_OK;
    }
    else if (!strcmp(pArray[2], "FAIL"))
    {
      status = RFID_FAIL;
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  }
  else if (!strcmp(pArray[1], "SHIP"))
  {
    if (!strcmp(pArray[2], "OK"))
    {
      // 카드 검증 완료
      status = SHIP_OK;
    }
    else if (!strcmp(pArray[2], "FAIL"))
    {
      status = SHIP_FAIL;
    }
    sprintf(sendBuf,"[%s]%s@%s\n",pArray[0],pArray[1],pArray[2]);
  }
  else
      return NONE;
  
  sendMsg(sendBuf);

#ifdef DEBUG
  Serial.print(", send : ");
  Serial.print(sendBuf);
#endif

  return status;
}
    
void Wifi::wifi_Setup() {

  wifiSerial.begin(19200);
  wifi_Init();
  server_Connect();
}
   
void Wifi::wifi_Init()
{
  do {
    WiFi.init(&wifiSerial);
    if (WiFi.status() == WL_NO_SHIELD) {
#ifdef DEBUG_WIFI    
      Serial.println("WiFi shield not present");
#endif 
    }
    else
      break;   
  }while(1);

#ifdef DEBUG_WIFI    
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(AP_SSID);
#endif     
  while(WiFi.begin(AP_SSID, AP_PASS) != WL_CONNECTED) {   
#ifdef DEBUG_WIFI  
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(AP_SSID);   
#endif   
  }
  
#ifdef DEBUG_WIFI      
  Serial.println("You're connected to the network");    
  printWifiStatus();
#endif 
}

int Wifi::server_Connect()
{
#ifdef DEBUG_WIFI     
  Serial.println("Starting connection to server...");
#endif  

  if (client.connect(SERVER_NAME, SERVER_PORT)) {
#ifdef DEBUG_WIFI     
    Serial.println("Connect to server");
#endif  
    client.print("["LOGID":"PASSWD"]"); 
  }
  else
  {
#ifdef DEBUG_WIFI      
     Serial.println("server connection failure");
#endif    
  } 
}
void Wifi::printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
