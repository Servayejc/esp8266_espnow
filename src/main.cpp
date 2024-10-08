#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Temperatures.h>
#include <esp_now_pairing.h>
#include <global.h>
#include <Ticker.h>
#include <map>
#include "control.h"

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

uint8_t relaySetPoint;
uint8_t relayStatus;

bool ledPair = false;


uint8_t defaultSP = 6;
float temp_sp = defaultSP;
uint8_t temp_SP[12] = {};  
uint8_t control[12] = {}; 
int curState[12] = {};

unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

unsigned long starting = millis();

bool deepSleepMode = false;

int sendInterval = 10;   // in seconds

uint8_t deviceTypes[3] = {THERMOSTAT, THERMOMETER, RELAY};
uint8_t modeTypes[2] = {DEEP_SLEAP_OFF, DEEP_SLEAP_ON};


struct_message myData;
struct_message setpoints;

int readingId = 0;
float temperature;
float Hysteresis = 0.5;
int count = 0;
bool ledState;

void sendDevice();
void sendAllDevices();

byte rtcStore[2];

Ticker timerSendAllDevices(sendAllDevices, sendInterval * 1000);   
Ticker timerSendDevice(sendDevice, 100);

void initSetPoints(int value){
  for (byte i = 0; i < 12; i++){
      temp_SP[i] = defaultSP;
      curState[i] = 0;
      control[i] = 0;
  }
}
void printSetpoint(){
    Serial.println("AFTER SETPOINT");
    Serial.print("SP : ");
    Serial.println(temp_SP[setpoints.deviceId]);
    Serial.print("STATUS : ");
    Serial.println(relayStatus);
}

void printData()
{
  Serial.println("-------------------");
  Serial.print("msgType : ");
  Serial.println(myData.msgType);
  Serial.print("ID : ");
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
  if (Sensors.size() > Ndx) {
    temperature = ReadTemp(Ndx);
    memcpy(myData.deviceAddress, Sensors[Ndx].Address,8);
  } else {
    temperature = 22.2;
  }
  myData.msgType = DATA;
  myData.deviceId = pairingData.deviceIds[Ndx];
  uint8_t deviceType = pairingData.deviceTypes[Ndx];
  myData.deviceType = deviceType;
  switch (deviceType)
  {
  case THERMOSTAT:
    if (temperature < temp_SP[Ndx] - Hysteresis)
    {
      curState[Ndx] = ON;
    }
    if (temperature > temp_SP[Ndx] + Hysteresis)
    {
      curState[Ndx] = OFF;
    }

    CTRL.setChannel(control[Ndx-1], curState[Ndx]);
    CTRL.setAuto(true);
    myData.F1 = temperature;
    myData.U1 = curState[Ndx];
    myData.F2 = temp_SP[Ndx];
    break;

  case THERMOMETER:
    myData.F1 = temperature;
    myData.U1 = curState[Ndx];
    break;

  case RELAY:
    myData.F1 = -1;
    myData.F2 = -1;
    myData.U1 = relayStatus;
    myData.U2 = relaySetPoint;
    break;

    case TH_ARRAY:
    myData.F1 = temperature;
    myData.F2 = -1;
    myData.U1 = -1;
    myData.U2 = -1;
    break;
  }
  esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
  //printData();
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

  case SETPOINTS:
    memcpy(&setpoints, incomingData, sizeof(setpoints));
    switch (setpoints.deviceType)
    {
    case THERMOSTAT: // thermostat

      temp_SP[setpoints.deviceId-1] = setpoints.U1;
      myData.U1 = temp_SP[setpoints.deviceId-1];
      break;
    
    case RELAY: // Relay
      relaySetPoint = setpoints.U1;
      relayStatus = setpoints.U1;
      digitalWrite(16, relayStatus); // 
      myData.U1 = relayStatus;
      break;
    }
    getReadings(setpoints.deviceId-1);
    // printSetPoint();
    esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
    break;
  }
}

void setup()
{
  Serial.begin(74880);
  
  pinMode(D5, OUTPUT);  
  digitalWrite(D5, OFF);
  pinMode(16, OUTPUT);
  digitalWrite(16, 0);

  pinMode(12, INPUT_PULLUP);
  deepSleepMode = !digitalRead(12);
     
  init_esp_now();
  register_recv_cb(&on_esp_now_data_recv);
  
  
  CTRL.test();

  searchAll();
  Serial.print("En of search : ");
  Serial.print(millis() - starting);
  Serial.println("ms");
  StartAllConversion();

  setpoints.F1 = defaultSP;
  setpoints.U1 = 0;
  
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
      timerSendDevice.stop();
      Serial.print("I'm awake from ");
      Serial.print(millis()- starting);
      Serial.print("ms , but I'm going into deep sleep mode for ");
      Serial.print(sendInterval);
      Serial.println(" seconds");
      ESP.deepSleep(sendInterval * 1000000);
    }
  } else {
    getReadings(timerSendDevice.counter()-1);
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
  if (autoPairing() == PAIR_PAIRED)
  {
    if (!deepSleepMode){ 
      timerSendAllDevices.update();
    }  
    timerSendDevice.update();
  }
}
