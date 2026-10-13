#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Temperatures.h>
#include <esp_now_pairing.h>
#include <global.h>
#include <Ticker.h>
#include <map>
#include "control.h"
#include "config.h"



//#include <MCP23017.h>

#include <Wire.h>

#include <EEPROM.h>

#define DEEP_SLEAP_OFF 0
#define DEEP_SLEAP_ON 1
#define ON 0
#define OFF 1
#define THERMOSTAT 0
#define THERMOMETER 1
#define RELAY 2
#define LEVEL 3
//#define TH_ARRAY 3

#define WIRE Wire

#define EEPROM_SIZE 1

ADC_MODE(ADC_VCC);

Control CTRL;

bool ledPair = false;

struct_dataRTC RTCdata = {};

unsigned long starting = millis();

bool deepSleepMode = false;

#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds

  

int sendInterval = 120;   // in seconds
int pairingInterval = 10; 


#define CONTROL  0x20


uint8_t deviceTypes[4]= {THERMOSTAT, THERMOMETER, RELAY, LEVEL};
uint8_t modeTypes[2] = {DEEP_SLEAP_OFF, DEEP_SLEAP_ON};

struct_message myData;
struct_message setpoints;

float temperature;

int count = 0;
bool ledState;

void sendDevice();
void sendAllDevices();



uint32_t rtt = 0;
uint32_t pingTimeLimit = 5000;
uint8_t pingErrorCounter = 0;

Ticker timerSendAllDevices(sendAllDevices, sendInterval * 1000);   
Ticker timerSendDevice(sendDevice, 100);

void printSetpoint(){
    /*
    Serial.println("AFTER SETPOINT");
    Serial.print("SP : ");
    Serial.println(RTCdata.setPoint[setpoints.deviceId]);
    Serial.print("STATUS : ");
    Serial.println(RTCdata.curState[setpoints.deviceId]);
    */
}

void printData()
{
  Serial.println("-------------------");
  Serial.print("msgType : ");
  Serial.println(myData.msgType);
  Serial.print("Device ID : ");
  Serial.println(myData.deviceId);
  Serial.print("DeviceType : ");
  Serial.println(myData.deviceType);
  
  Serial.print("Sensor Address : ");
  for ( byte i = 0; i < 8; i++) {
  Serial.print(myData.deviceAddress[i],HEX);
  Serial.print(" ");
  }
  Serial.println();

  Serial.print("F1 : ");
  Serial.println(myData.F1);
  Serial.print("F2 : ");
  Serial.println(myData.F2);
  Serial.print("U1 : ");
  Serial.println(myData.U1);
  Serial.print("U2 : ");
  Serial.println(myData.U1);
  Serial.println("-------------------");
}

void getReadings(uint8_t Ndx)
{    
 
  //uint8_t level = CTRL.getLevel();
  //Serial.print("Level --> : ");
  //Serial.println(level); 
  Serial.println("Get Readings for Ndx : " + String(Ndx));
  
  if (Sensors.size() > Ndx) {
    temperature = ReadTemp(Ndx);
    memcpy(myData.deviceAddress, Sensors[Ndx].Address,8);
  } else {
    temperature = 88.88;
  }
  Serial.print("Temperature : ");
  Serial.println(temperature);
  myData.msgType = DATA;
  myData.deviceId = pairingData.deviceIds[Ndx];
  uint8_t deviceType = pairingData.deviceTypes[Ndx];
  myData.deviceType = deviceType;
  //deviceType = 3;
  switch (deviceType)
  {
  case THERMOSTAT:
    if (temperature < RTCdata.setPoint[Ndx] - 0.5)
    {
       RTCdata.curState[Ndx] = ON;
    }
    if (temperature > RTCdata.setPoint[Ndx] + 0.5)
    {
       RTCdata.curState[Ndx] = OFF;
    }
    CTRL.setCtrlChannel(pairingData.controlNdx[myData.deviceId-1], (bool)RTCdata.curState[Ndx]);
    //CTRL.setAuto(true);
    myData.F1 = temperature;
    myData.U1 = (uint8_t)RTCdata.curState[Ndx];
    myData.F2 = RTCdata.setPoint[Ndx];
    myData.U5 = rtt;
    break;

  case THERMOMETER:
    myData.F1 = temperature;
    myData.U1 = (uint8_t)RTCdata.curState[Ndx];
    break;

  case RELAY:
    myData.F1 = -1;
    myData.F2 = -1;
    myData.U1 = (uint8_t)RTCdata.setPoint[Ndx];
    myData.U2 = (uint8_t)RTCdata.curState[Ndx];;
    break;

    case LEVEL:
    myData.F1 = CTRL.getLevel();
    myData.F2 = CTRL.getLevel();
    myData.U1 = -1;
    myData.U2 = -1;
    break;
  }
  esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
  printData();
}

void on_esp_now_data_sent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.println();
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
  }
}

void on_esp_now_data_recv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  uint8_t type = incomingData[0];
  switch (type)
  {
  case DATA:
    break;

  case PING:  
    if (rtt > pingTimeLimit) {  // 5sec
      pingErrorCounter ++;
      #ifdef DEBUG_PING
        Serial.println("Ping skipped, RTT is still high");
      #endif
        Serial.println("Ping failed, resetting pairing");
        resetPairing();
      } else {
        memcpy(&pingData, incomingData, sizeof(pingData));
        rtt = millis() - pingData.pingTime;
        myData.U5 = (uint8_t)rtt;
        #ifdef DEBUG_PING 
          Serial.print("Pong received, RTT : ");
          Serial.print(rtt);
          Serial.println(" ms");
        #endif
    }
    break;
    
  case SETPOINTS:
    memcpy(&setpoints, incomingData, sizeof(setpoints));
    switch (setpoints.deviceType)
    {
    case THERMOSTAT: // thermostat

      RTCdata.setPoint[setpoints.deviceId-1] = setpoints.U1;
      myData.U1 = RTCdata.setPoint[setpoints.deviceId-1];
      break;
    
    case RELAY: // Relay
      digitalWrite(16, RTCdata.curState[setpoints.deviceId-1]); // 
      myData.U1 = RTCdata.curState[setpoints.deviceId-1];
      break;
    }
    saveRTCdata(false); 
    getReadings(setpoints.deviceId-1);
    
    readRTCdata();
    esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
    break;
  }
}

/*
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  */
     

void setup()
{
  
  Serial.begin(74880);
  Serial.println();
  Serial.println("Starting 1");
  rst_info *resetInfo = ESP.getResetInfoPtr();
  Serial.println("Reset reason : " + String(resetInfo->reason));
  //pinMode(D0, OUTPUT);
  //digitalWrite(D0, HIGH);
 // saveRTCdata(true);

  pingErrorCounter = 0;
  pinMode(12, INPUT_PULLUP);
  deepSleepMode = 1;//!digitalRead(12);
  readRTCdata();
  
  
  /*Serial.begin(74880);
  Serial.println();
  Serial.println("Starting 2");*/
  //Serial.println("Board ID : " + String(BOARD_ID));
  
  //CTRL.resetMCP017();
   
  //pinMode(12, INPUT_PULLUP);
  //deepSleepMode = !digitalRead(12);

  init_esp_now();
  register_recv_cb(&on_esp_now_data_recv);
  setBoardIOs();
  setupOneWireBus();
 
  Serial.print("End of search : ");
  Serial.print(millis() - starting);
  Serial.println("ms");
  StartAllConversion();

  //CTRL.initLevel();
  //setpoints.F1 = defaultSP;
  //setpoints.U1 = 0;
  
  if (deepSleepMode)  { 
    timerSendDevice.start();
  } else {
    timerSendAllDevices.start();
  }
  Serial.println("Setup done");
}

void sendDevice()       //100 ms
{ 
  if (pairingData.deviceTypes[timerSendDevice.counter()-1] == 255){
    timerSendDevice.stop();
    StartAllConversion();
    if (deepSleepMode){ 
      Serial.println("DeepSleep mode ON");
      timerSendDevice.stop();
      Serial.print("I'm awake from ");
      Serial.print(millis()- starting);
      Serial.print("ms, ");
      doDeepSleepSec(sendInterval); // 10 seconds
      //doDeepSleepSec(3600); 
    }
  } else {
    Serial.println("DeepSleep mode OFF");
    getReadings(timerSendDevice.counter()-1);
  }
}

void sendAllDevices()     // 500 ms
{
  timerSendDevice.start();
  starting = millis();
  Serial.print(F("--- Heap size : "));
  Serial.println(ESP.getFreeHeap());
  ping();
}

void loop()
{  
  if (autoPairing() == PAIR_PAIRED)
  {
    if (!deepSleepMode){ 
      timerSendAllDevices.update();
    }  
    timerSendDevice.update(); 
  } 
}


 
