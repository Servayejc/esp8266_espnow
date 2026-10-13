#include <esp_now_pairing.h>
#include <global.h>
#include <Ticker.h>
#include "Config.h"
#include "Config.h"

#include <EEPROM.h>

uint16_t validity = 4583;
// callback for receiving data type DATA
void(*recv_cb)(uint8_t * mac, uint8_t *incomingData, uint8_t len) = NULL;
uint8_t broadcastAddressX[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t serverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

//void pairingPause();
//Ticker timerPairingPause(pairingPause, 3600*1000);

PairingStatus pairingStatus = PAIR_REQUEST;
int channelCount = 0;
MessageType messageType;

unsigned long currentMillis = millis();
unsigned long previousMillis = 0;

struct_pairing pairingData;
struct_ping pingData;

int errCount = 0;
unsigned long start;
uint8_t channel = 1;



void saveRTCdata(bool init){
//ESP.rtcUserMemoryWrite(0, &counter, sizeof(counter)); 

  RTCdata.validity = validity;
    if (init) {  
        for (byte i = 0; i < 12; i++){
            RTCdata.setPoint[i] = 20;
            RTCdata.curState[i] = 0;
        }
        RTCdata.channel = 1;
        RTCdata.deepSleepCount = DEEP_SLEEP_COUNT;
    }
   system_rtc_mem_write(65, &RTCdata, sizeof(RTCdata));
    //EEPROM.put(0,RTCdata);
    //EEPROM.commit();
}

void readRTCdata(){
  // TODO Read data from RTC Mem
  Serial.print("RTCdata size : "); 
  Serial.println(sizeof(RTCdata));
  system_rtc_mem_read(65, &RTCdata, sizeof(RTCdata)); 
  //EEPROM.begin(512);
  //EEPROM.get(0, RTCdata);
    Serial.print("Validity : " );
    Serial.println(RTCdata.validity);
 if (RTCdata.validity != validity) { 
    Serial.println("RTCdata not present"); 
    saveRTCdata(true);
    EEPROM.get(0, RTCdata); 
    //ESP.rtcUserMemoryRead (16, (uint32_t*) &RTCdata , sizeof(RTCdata ));
  }else{
    Serial.println("RTCdata present");
    Serial.print("Validity : " );
    Serial.println(RTCdata.validity);
    Serial.print("Channel : " );
    Serial.println(RTCdata.channel);
    Serial.print("DeepSleepCount : ");
    Serial.println(RTCdata.deepSleepCount);
   
    /*
    for (byte i = 0; i < 12; i++){
        Serial.print(" [ ");
        Serial.print(RTCdata.setPoint[i]);
        Serial.print(", ");
        Serial.print(RTCdata.curState[i]);
        Serial.print(" ] ");
        Serial.print("-");
    }
    Serial.println();
    */
  }  
}

void setChannelFromRTC(){
  channel = RTCdata.channel;
  Serial.print("Channel set from RTC : ");
  Serial.println(channel);
}


//uint32_t pingTimeLimit = 5000;

// send ping to servewr
void ping(){
  /*if (rtt > pingTimeLimit) {  
    #ifdef DEBUG_PING
      Serial.println("Ping skipped, RTT is still high");
    #endif
    pairingStatus = PAIR_REQUEST;  // add a counter of failed
    return;
  }*/
  pingData.msgType = PING;
  pingData.PeerID = BOARD_ID;
  pingData.pingTime = millis();
  esp_now_send(serverAddress, (uint8_t *) &pingData, sizeof(pingData));
  #ifdef DEBUG_PING
    Serial.println("Ping Sended");
  #endif
}

int getChannel()
{
    uint32_t result = 0;
    ESP.rtcUserMemoryRead(0, &result, 1);
    if (result < 1) result = 1;
    if (result > 11) result = 1;
    return (int)result;
}

void setChannel(int value)
{
    uint32_t storeValue = value;
    ESP.rtcUserMemoryWrite(0, &storeValue, 1);
}

void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  macStr[17] = '\0';
  Serial.println(macStr);
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  #ifdef DEBUG_SEND_DATA
    Serial.print("Last Packet Send Status: ");
    if (sendStatus == 0){
      Serial.print("Delivery success to ");
      printMAC(mac_addr);
      //Serial.println();
    }
  #endif
}

void printPairingData(){
  #ifdef DEBUG_PAIRING_DATA
    Serial.println( "=================== Pairing Data ===============");
    Serial.print("msgType : "); 
    Serial.println(pairingData.msgType);
    Serial.print("netWork : "); 
    Serial.println(pairingData.network);
    Serial.print("macAddr : "); 
    for(int i = 0; i < 6; i++){
        Serial.print(pairingData.macAddr[i],HEX);
        Serial.print(", ");
    } 
    Serial.println();
    Serial.print("Channel : ");
    Serial.println(pairingData.channel);
        
    Serial.print("deviceTypes : ");  
    for(int i = 0; i < 6; i++){
        Serial.print(pairingData.deviceTypes[i]);
        Serial.print(", ");
    } 
    Serial.println(); 
    Serial.print("deviceId : ");
    for(int i = 0; i < 6; i++){
        Serial.print(pairingData.deviceIds[i]);
        Serial.print(", ");
      } 
    Serial.println();
    Serial.print("controlNdx : ");
    for(int i = 0; i < 6; i++){
        Serial.print(pairingData.controlNdx[i]);
        Serial.print(", ");
      } 
    Serial.println();
    
  #endif
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  
  uint8_t type = incomingData[0];
  #ifdef DEBUG_INCOMING_DATA
    
 /* Serial.print("Size of message : ");
    Serial.print(len);
    Serial.print(" - Type: ");
    Serial.println(type);


    Serial.print(" from ");
    printMAC(mac);
    Serial.println();
    Serial.print("MSG Type : "); 
    Serial.println(type); */
  #endif

  switch (type) {
  case DATA :
    (recv_cb)(mac, incomingData, len);  
    break;

  case SETPOINTS :
    (recv_cb)(mac, incomingData, len);  
    break;  
  
  case RESET:
    Serial.println("=================== Reset Pairing ===============");
    if (incomingData[1] == SERVER_ID){ 
      resetPairing();
      digitalWrite(16, 0);
    }  
    break;
  
  case PAIRING:
    Serial.print("Pairing data length = ");
    Serial.println(len);
    printPairingData();
    memcpy (&pairingData, incomingData, sizeof(pairingData));
    Serial.print("Server ID : "); 
    Serial.println(pairingData.id);
    if (true){// (len==46){
      if (pairingData.network == SERVER_ID){                // the message comes from server
        #ifdef DEBUG_PAIRING
          Serial.print("Pairing done on channel ");
          Serial.print(pairingData.channel);    // channel used by the server
          Serial.print(" in ");
          Serial.print(millis()-start);
          Serial.print("ms");
          Serial.print(" for ");
          printMAC(pairingData.macAddr);
        #endif
    
        memcpy(serverAddress, pairingData.macAddr, 6);
        esp_now_add_peer(pairingData.macAddr, ESP_NOW_ROLE_COMBO, pairingData.channel, NULL, 0); // add the server to the peer list 
        setChannel(pairingData.channel);
        pairingStatus = PAIR_PAIRED ;
        // set the pairing status
        //save channel in RTC memory
        RTCdata.channel = pairingData.channel;
        saveRTCdata(false);
        Serial.println("Pairing done");
      }
    } else {
      Serial.print("Wrong pairing message received");
      printMAC(mac);
      Serial.println();
    }  
    break;
  
  } 
}

void resetPairing(){
   pairingStatus = PAIR_REQUEST;
}


PairingStatus autoPairing(){
  //Serial.print(digitalRead(12));
  switch(pairingStatus) {
  case PAIR_REQUEST:
    
    Serial.print("Pairing request on channel "  );
    Serial.println(channel);
  
    // clean esp now
    esp_now_deinit();
    WiFi.mode(WIFI_STA);
    // set WiFi channel   
    wifi_promiscuous_enable(1);
    wifi_set_channel(channel);
    wifi_promiscuous_enable(0);
    WiFi.disconnect();

    // Init ESP-NOW
    if (esp_now_init() != 0) {
      Serial.println("Error initializing ESP-NOW");
    }
    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    // set callback routines
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv);
    esp_now_add_peer(broadcastAddressX, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
    
    // set pairing data to send to the server
    
    pairingData.msgType = PAIRING;
    pairingData.network = SERVER_ID;
    pairingData.id = BOARD_ID;     
    pairingData.channel = channel;


    previousMillis = millis();
    printPairingData();
    
    // add peer and send request
    esp_now_send(broadcastAddressX, (uint8_t *) &pairingData, sizeof(pairingData));
    //previousMillis = millis();
    Serial.println("Pairing requested");
    pairingStatus = PAIR_REQUESTED;
    break;

  case PAIR_REQUESTED:
    //Serial.println("Requested");
    // time out to allow receiving response from server
    
    currentMillis = millis();
    if(currentMillis - previousMillis > 500) {
      previousMillis = currentMillis;
      // time out expired,  try next channel
      channel ++;
      if (channel > 11) {
        channel = 1;
      }
      channelCount ++;
      if (channelCount > 11*2){
        pairingStatus = PAIR_PAUSED;
      } else {  
        pairingStatus = PAIR_REQUEST;
      }
    }
    break;

  case PAIR_PAUSED:
    Serial.println("Pairing paused");
    doDeepSleepSec(3600); // 1 hour
  break;  
  

  case PAIR_PAIRED:
    break;
  }

  return pairingStatus;
} 

void init_esp_now() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.print("Board ID : ");
  Serial.println(BOARD_ID);
  Serial.print("Server ID : ");
  Serial.println(SERVER_ID);
  Serial.print("Mac Address : ");
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
    
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  channel = getChannel();
}

uint8_t getDevicesCount(){
   uint8_t count = 0;
   for(int i = 0; i < 12; i++){
        if (pairingData.deviceTypes[i] < 255){
          count++;
        }
   }
   return count;     
}

void register_recv_cb(void(*Callback_ptr)(uint8_t * mac, uint8_t *incomingData, uint8_t len)) {
  recv_cb = Callback_ptr;
}



