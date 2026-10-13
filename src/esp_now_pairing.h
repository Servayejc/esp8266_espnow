#ifndef ESPNOW_AUTOPAIRING_H_
#define ESPNOW_AUTOPAIRING_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <global.h>


//#define BOARD_ID 2


extern uint8_t serverAddress[6];

void register_recv_cb(void(*Callback_ptr)(uint8_t * mac, uint8_t *incomingData, uint8_t len));
void init_esp_now();
void printMAC(const uint8_t * mac_addr);
uint8_t getDevicesCount();
PairingStatus autoPairing();
void resetPairing();
void ping();
void readRTCdata();
void saveRTCdata(bool init);
//void pairingInterval(int interval);

class Pairing  {      
  private:
    uint8_t serverAddress[6];
    uint8_t channel = 1;
    
    
   // status pairingStatus = PAIR_REQUEST; 
  public: 
    Pairing();  
    uint32_t pingTimeLimit;          
    void init_esp_now();
    void resetPairing();
    PairingStatus autoPairing();
    void printPairingData();
    void setChannelFromRTC();
    
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