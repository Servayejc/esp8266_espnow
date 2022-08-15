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
uint8_t broadcastAddress[] = {0x78, 0xE3, 0x6D, 0x09, 0xEE, 0x84};

// Structure example to send data
// Must match the receiver structure


typedef struct struct_message {
  int id;
    float temp;
    float hum;
    int readingId;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 2000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(74880);
    // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  wifi_promiscuous_enable(1);
  wifi_set_channel(11);
  //wifi_promiscuous_enable(0);
  WiFi.printDiag(Serial);
   
  
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 11, NULL, 0);
  
}
 
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    myData.hum = 50;
    myData.temp = 22;
    myData.id = 55;
    myData.readingId = 2;

    // Send message via ESP-NOW
    esp_now_send(NULL, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();
  }
}