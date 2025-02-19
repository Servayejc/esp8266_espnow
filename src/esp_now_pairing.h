#ifndef ESPNOW_AUTOPAIRING_H_
#define ESPNOW_AUTOPAIRING_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <global.h>

extern uint8_t serverAddress[6];

void register_recv_cb(void(*Callback_ptr)(uint8_t * mac, uint8_t *incomingData, uint8_t len));
void init_esp_now(uint8_t WiFiChannel);
void printMAC(const uint8_t * mac_addr);
uint8_t getDevicesCount();
PairingStatus autoPairing();
void resetPairing();



#endif