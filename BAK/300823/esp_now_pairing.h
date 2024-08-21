#ifndef ESPNOW_AUTOPAIRING_H_
#define ESPNOW_AUTOPAIRING_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <global.h>

#define SERVER_ID 99
#define BOARD_ID 3


//enum PairingStatus {PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED, };
//enum MessageType {PAIRING, DATA, SETPOINTS, PING, };
//enum Types {}
extern uint8_t serverAddress[6];

/*typedef struct struct_pairing 
{           
    uint8_t msgType;
    uint8_t network;
    uint8_t id;
    uint8_t macAddr[6];
    uint8_t channel;
} struct_pairing;

typedef struct struct_ping
{
  uint8_t msgType;
  int msgID;
} struct_ping;
*/

void register_recv_cb(void(*Callback_ptr)(uint8_t * mac, uint8_t *incomingData, uint8_t len));
void init_esp_now();
void printMAC(const uint8_t * mac_addr);
uint8_t getDevicesCount();
PairingStatus autoPairing();

#endif