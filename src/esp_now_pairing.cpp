#include <esp_now_pairing.h>
#include <global.h>
#include <Ticker.h>
#include "Config.h"



// callback for receiving data type DATA
void(*recv_cb)(uint8_t * mac, uint8_t *incomingData, uint8_t len) = NULL;
uint8_t broadcastAddressX[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t serverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void pairingPause();
Ticker timerPairingPause(pairingPause, 10000); // 10 seconds

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



int getChannel()
{
    uint32_t result;
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
  Serial.println(macStr);
}

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  //printMAC(mac_addr);
  //Serial.println();
  //Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    //Serial.println("Delivery success");
    errCount = 0;
  }
  else{
    Serial.println("Delivery fail");
    if (autoPairing() == PAIR_PAIRED){
        errCount++;
        if (errCount > 3) {
          Serial.println("Sending error, server may be restarted on new channel");
          pairingStatus = PAIR_REQUEST;
          errCount = 0; 
        }
    }   
  }
}

void printPairingData(){
  #ifdef DEBUG_PAIRING_DATA
    Serial.print("msgType : "); 
    Serial.println(pairingData.msgType);
    Serial.print("netWork : "); 
    Serial.println(pairingData.network);
    Serial.print("macAddr : "); 
    for(int i = 0; i < 6; i++){
        Serial.print(pairingData.macAddr[i]);
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
    Serial.print("Size of message : ");
    Serial.print(len);
    Serial.print(" from ");
    printMAC(mac);
    Serial.println();
    Serial.print("MSG Type : "); 
    Serial.println(type); 
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
    if (incomingData[1] == SERVER_ID){  //
      //resetPairing();
      ESP.restart();
    }  
    break;
  
  case PAIRING:
    Serial.print("--- Pairing to ");
    Serial.print(pairingData.id);
    Serial.println(" ---"); 
    memcpy (&pairingData, incomingData, sizeof(pairingData));
    
    if (pairingData.id == SERVER_ID) {                // the message comes from server
      #ifdef DEBUG_PAIRING
        Serial.print("Pairing done on channel ");
        Serial.print(pairingData.channel);    // channel used by the server
        Serial.print(" in ");
        Serial.print(millis()-start);
        Serial.print("ms");
        Serial.print(" for ");
        printMAC(pairingData.macAddr);
      #endif
      
      
      for(int i = 0; i < 12; i++){
        RTCdata.control[i] = pairingData.controlNdx[i]; 
      }

      printPairingData();
   
      memcpy(serverAddress, pairingData.macAddr, 6);
      esp_now_add_peer(pairingData.macAddr, ESP_NOW_ROLE_COMBO, pairingData.channel, NULL, 0); // add the server to the peer list 
      setChannel(channel);
      pairingStatus = PAIR_PAIRED ;
                  // set the pairing status
    }
    break;
    
    case PING:
      memcpy (&pingData, incomingData, sizeof(pingData));
      Serial.println("Ping Received");
      esp_now_send(serverAddress, (uint8_t *) &pingData, sizeof(pingData));
      Serial.println("Ping Responded");
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
    
    // set pairing data to send to the server
    
    pairingData.msgType = PAIRING;
    pairingData.network = SERVER_ID;
    pairingData.id = BOARD_ID;     
    pairingData.channel = channel;

    previousMillis = millis();

    
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
        timerPairingPause.start();
        Serial.println("Pairing paused");
        pairingStatus = PAIR_PAUSED;
      } else {  
        pairingStatus = PAIR_REQUEST;
      }
    }
    break;

  case PAIR_PAUSED:
    timerPairingPause.update();
    break;  

  case PAIR_PAIRED:
    //Serial.println("Paired!");
    digitalWrite(16, 1);
    break;
  }
  return pairingStatus;
} 

void pairingPause(){
  if (timerPairingPause.elapsed()){
    Serial.println("Pause Elapsed");
    timerPairingPause.stop();
    channelCount = 0;
    pairingStatus = PAIR_REQUEST;
  }  
}   

void init_esp_now() {
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.print("Board ID :");
  Serial.println(BOARD_ID);
  Serial.print("Server ID :");
  Serial.println(SERVER_ID);
  Serial.print("Mac Address :");
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



