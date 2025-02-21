#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Temperatures.h>
#include <esp_now_pairing.h>
#include <global.h>
#include <Ticker.h>
#include <map>
#include "control.h"
#include "config.h"
#include "utils.h"

#include <EEPROM.h>

#define DEEP_SLEAP_OFF 0
#define DEEP_SLEAP_ON 1
#define ON 0
#define OFF 1
#define THERMOSTAT 0
#define THERMOMETER 1
#define RELAY 2
#define TH_ARRAY 3
#define EEPROM_SIZE 1

ADC_MODE(ADC_VCC);

Control CTRL;
Temps TEMPS;

bool ledPair = false;

unsigned long starting = millis();

bool deepSleepMode = false;
int sendInterval = 10;   // in seconds

uint8_t deviceTypes[3] = {THERMOSTAT, THERMOMETER, RELAY};
uint8_t modeTypes[2] = {DEEP_SLEAP_OFF, DEEP_SLEAP_ON};


struct_message myData;
struct_message setpoints;

bool ledState;

void sendDevice();
void sendAllDevices();


Ticker timerSendAllDevices(sendAllDevices, sendInterval * 1000);   
Ticker timerSendDevice(sendDevice, 100);

void on_esp_now_data_sent(uint8_t *mac_addr, uint8_t sendStatus)
{
  Serial.println();
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
  }
}

void on_esp_now_data_recv(uint8_t *mac, uint8_t *incomingData, uint8_t len )
{
  uint8_t type = incomingData[0];
  switch (type)
  {
  case DATA:
    break;

  case SETPOINTS:
    memcpy(&setpoints, incomingData, sizeof(setpoints));
    switch (setpoints.deviceType)
    {
    case THERMOSTAT: // thermostat

      RTCdata.temp_SP[setpoints.deviceId-1] = setpoints.U1;
      myData.U1 = RTCdata.temp_SP[setpoints.deviceId-1];
      break;
    
    case RELAY: // Relay
    //  relaySetPoint = setpoints.U1;
   //   relayStatus = setpoints.U1;
    //  digitalWrite(16, relayStatus); // 
     // myData.U1 = relayStatus;
      break;
    }
    TEMPS.getReadings(setpoints.deviceId-1, &myData);
    esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
    break;
  }
}

void setup()
{
  Serial.begin(74880);
  
  pinMode(D8, OUTPUT);  
  digitalWrite(D8, 0);

 // retrieve important data after power failure
  readRTCdata();
  RTCdata.validity = RTCdata.validity + 1;
  saveRTCdata(false);
 
   
  // register esp_now data_reed callback  
  init_esp_now(RTCdata.WiFiChannel);   
 

  // test control board
  CTRL.test();

  //search connected 1-wire devices  
  TEMPS.searchAll();
  Serial.print("En of search : ");
  Serial.print(millis() - starting);
  Serial.println("ms");
  
  // read all 1-wire data
  TEMPS.StartAllConversion();
  
  //Deep sleep control
  pinMode(12, INPUT_PULLUP);
  deepSleepMode = !digitalRead(12);
  Serial.println(deepSleepMode);
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
    TEMPS.StartAllConversion();
    if (deepSleepMode){ 
      timerSendDevice.stop();
      Serial.print("I'm awake from ");
      Serial.print(millis()- starting);
      Serial.print("ms , but I'm going into deep sleep mode for ");
      Serial.print(sendInterval);
      Serial.println(" seconds");
      ESP.deepSleep(sendInterval * 1000000);
    }
  } else {
    updateSimulation();
    TEMPS.getReadings(timerSendDevice.counter()-1, &myData);
    esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
  }
}

void sendAllDevices()     // 500 ms
{
  timerSendDevice.start();
  starting = millis();
  Serial.print(F("--- Heap size : "));
  Serial.println(ESP.getFreeHeap());
}

void loop()
{   
  register_recv_cb(&on_esp_now_data_recv);
  if (autoPairing() == PAIR_PAIRED)
  {
    if (!deepSleepMode){ 
      timerSendAllDevices.update();
    }  
    timerSendDevice.update();
  }
  
}
