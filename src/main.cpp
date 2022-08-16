/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
#include <arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>

// REPLACE WITH RECEIVER MAC Address
//uint8_t broadcastAddress[] = {0x78, 0xE3, 0x6D, 0x09, 0xEE, 0x84};
//uint8_t broadcastAddress[] = {0x84, 0xf3, 0xeb, 0xee, 0x42, 0xce};
//uint8_t broadcastAddress[] = {0x84, 0xf3, 0xeb, 0x80, 0xef, 0xda};
uint8_t serverAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int channel = 1;

enum PairingStatus {PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED, };
PairingStatus pairingStatus = PAIR_REQUEST;

unsigned long previousMillis = 0; 
unsigned long start;
typedef struct struct_message {
    int id;
    float temp;
    float hum;
    int readingId;
} struct_message;

// structure size must be different from message size
typedef struct struct_pairing {
    int id;
    int channel;
} struct_pairing;

// Create a struct_message called myData
struct_message myData;
struct_message incomingReadings;
struct_pairing pairingData;

#define BOARD_ID 2
#define SERVER_ID 0

unsigned long currentMillis = millis(); 
unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

void printMAC(const uint8_t * mac_addr){
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
}

void addPeer(uint8_t * mac_addr, int chan){
  esp_now_del_peer(serverAddress);
  esp_now_add_peer(mac_addr, ESP_NOW_ROLE_COMBO, chan, NULL, 0);
  memcpy(serverAddress, mac_addr, sizeof(uint8_t[6]));
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  if (len == sizeof(myData)){    
    memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.println(incomingReadings.id);
    Serial.println(incomingReadings.temp);
    Serial.println(incomingReadings.hum);
    Serial.println(incomingReadings.readingId);
  }
  if (len == sizeof(pairingData)){  // we received pairing request from server
    memcpy(&pairingData, incomingData, sizeof(pairingData));
    if (pairingData.id == SERVER_ID) {                // the message comes from server
      Serial.print("Pairing done for ");
      printMAC(mac_addr);
      Serial.print(" on channel " );
      Serial.print(pairingData.channel);    // channel used by the server
      Serial.print(" in ");
      Serial.print(millis()-start);
      Serial.println("ms");
      addPeer(mac_addr, pairingData.channel); // add the server to the peer list 
      pairingStatus = PAIR_PAIRED;
      
                  // set the pairing status
    }
  }  
}

void setup() {
  // Init Serial Monitor
  Serial.begin(74880);
  
  myData.id = 200;
  myData.hum = 50;
  myData.temp = 22;
  myData.readingId = 100;

  pairingStatus = PAIR_REQUEST;
  start = millis();
}

PairingStatus autoPairing(){
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
    //WiFi.printDiag(Serial);
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
    pairingData.id = BOARD_ID;     
    pairingData.channel = channel;

    // add peer and send request
    esp_now_del_peer(serverAddress);
    addPeer(serverAddress, channel);
    esp_now_send(serverAddress, (uint8_t *) &pairingData, sizeof(pairingData));
    pairingStatus = PAIR_REQUESTED;
    break;

  case PAIR_REQUESTED:
    // time out to allow receiving response from server
    currentMillis = millis();
    if(currentMillis - previousMillis > 50) {
      previousMillis = currentMillis;
      // time out expited,  try next channel
      channel ++;
      if (channel > 12) {channel = 0;}
      pairingStatus = PAIR_REQUEST; 
    }
    break;

  case PAIR_PAIRED:
    //Serial.println("Paired!");
    break;
  }
  return pairingStatus;
} 

void loop() {
  if (autoPairing() == PAIR_PAIRED) { 
    if ((millis() - lastTime) > timerDelay) {
      // Send message via ESP-NOW
      Serial.print("Sending message to ");
      printMAC(serverAddress);
      Serial.print(" on port ");
      Serial.println(channel);
     // addPeer(serverAddress, channel);
      esp_now_send(serverAddress, (uint8_t *) &myData, sizeof(myData));
      myData.readingId++;
      lastTime = millis();
    }  
  }
}