#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Temperatures.h>
#include <Common.h>
#include <esp_now_pairing.h>
#include <global.h>
#include <Ticker.h>
#include <map>




#define ON 0
#define OFF 1
#define THERMOSTAT 0
#define THERMOMETER 1
#define RELAY 2
#define NORMAL 0
#define BATTERY 1

uint8_t relaySetPoint;
uint8_t relayStatus;

float defaultSP = 9;
float temp_sp = defaultSP;

unsigned long starting = millis();

bool deepSleepMode = true;
int sendInterval = 10;   // in seconds

uint8_t deviceTypes[3] = {THERMOSTAT, THERMOMETER, RELAY};

struct_message myData;
struct_message setpoints;

int readingId = 0;

float temperature;

int curState = 0;

float Hysteresis = 0.5;

int count = 0;

bool ledState;

void sendDevice();
void sendAllDevices();

Ticker timerSendAllDevices(sendAllDevices, sendInterval * 1000);   
Ticker timerSendDevice(sendDevice, 100);

void printData()
{
  Serial.println("-------------------");
  Serial.print("msgType : ");
  Serial.println(myData.msgType);
  Serial.print("ID : ");
  Serial.println(myData.deviceId);
  Serial.print("DeviceType : ");
  Serial.println(myData.deviceType);
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
  if (Sensors.size() > 0) {
    temperature = ReadTemp(0);
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
    if (temperature < temp_sp - Hysteresis)
    {
      curState = ON;
    }
    if (temperature > temp_sp + Hysteresis)
    {
      curState = OFF;
    }
    myData.F1 = temperature;
    myData.U1 = curState;
    myData.F2 = temp_sp;
    break;

  case THERMOMETER:
    myData.F1 = temperature + 5;
    myData.U1 = curState;
    break;

  case RELAY:
    myData.F1 = -1;
    myData.F2 = -1;
    myData.U1 = relayStatus;
    myData.U2 = relaySetPoint;
    break;
  }
  // printData();
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
    Serial.print("------------New setpoint received : ");
    Serial.println();
    Serial.println(setpoints.msgType);
    Serial.println(setpoints.deviceId);
    Serial.println(setpoints.deviceType);
    Serial.println(setpoints.U1);

    switch (setpoints.deviceType)
    {
    case THERMOSTAT: // thermostat
      temp_sp = setpoints.U1;
      break;
    case RELAY: // Relay
      relaySetPoint = setpoints.U1;
      relayStatus = setpoints.U1;
      digitalWrite(16, relayStatus);
      myData.U1 = relayStatus;
      break;
    }
    getReadings(setpoints.deviceId);
    Serial.println("AFTER SETPOINT");
    Serial.print("SP : ");
    Serial.println(temp_sp);
    Serial.print("STATUS : ");
    Serial.println(relayStatus);
    esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
    break;
  }
}

void setup()
{
  Serial.begin(74880);

  pinMode(D5, OUTPUT);        // ???
  digitalWrite(D5, OFF);
  //pinMode(LED_BUILTIN, OUTPUT);
  // pinMode(16, OUTPUT);

  init_esp_now();
  register_recv_cb(&on_esp_now_data_recv);

  Serial.println(search());
  Serial.print("En of search : ");
  Serial.print(millis() - starting);
  Serial.println("ms");
  StartConversion();

  setpoints.F1 = defaultSP;
  setpoints.U1 = 0;
  
  if (deepSleepMode){ 
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
    Serial.print("Send data of device ");
    Serial.println(timerSendDevice.counter());
    getReadings(timerSendDevice.counter()-1);
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
  
  if (autoPairing() == PAIR_PAIRED)
  {
    if (!deepSleepMode){ 
      timerSendAllDevices.update();
    }  
    timerSendDevice.update();
  }
}
