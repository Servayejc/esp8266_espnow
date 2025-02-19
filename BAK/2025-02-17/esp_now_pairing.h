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


class Pairing  {      
  private:
    uint8_t serverAddress[6];
    uint8_t channel = 1;
   // status pairingStatus = PAIR_REQUEST; 
  public: 
    Pairing();            
    void init_esp_now(uint8_t WiFiChannel);
    void resetPairing();
    PairingStatus autoPairing();
	/*bool present();
	
	uint8_t getStatus();
	void setChannel(uint8_t channel, bool value);
	//void setOn(uint8_t channel);
	//void setOff(uint8_t channel);
	void setAuto(bool state);
	bool isAuto();*/
};

//extern Control CTRL;



#endif