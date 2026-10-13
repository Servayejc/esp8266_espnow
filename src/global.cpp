#include "global.h"

#include <vector>

    
uint8_t CONTROL_PIN;
uint8_t DEEP_SLEEP_PIN;
oneWirePinList oneWirePins = {};

void doDeepSleepSec(int seconds){
  Serial.print("Going into deep sleep mode for ");
  if (seconds < 60) {
    Serial.print(seconds);
    Serial.println(" seconds");
  } else {
    Serial.print(seconds/60 );
    Serial.println(" minutes");
  }
  ESP.deepSleep(seconds * 1000000ULL); 
}

void setBoardIOs(){
    uint8_t board = BOARD_ID;
    Serial.print("Board ID : ");
    Serial.println(board);
    switch (board) {
    // Chickens temperature
    case 2: 
        oneWirePins.push_back(D6);
        break;
    
    // House Heating with control board
    case 3:
        CTRL_SDA = D1;             //PIO5
        CTRL_SCL = D2;             //PIO4
        oneWirePins.push_back(D3); //PIO0
        oneWirePins.push_back(D4); //PIO2
        break;
   
    // Pool temperature
    case 4: 
        oneWirePins.push_back(D2); //PIO4
        DEEP_SLEEP_PIN = D6;       //PIO12    
        break;
    
    // Water Level
    case 5: 
     oneWirePins.push_back(D6);   
    // CTRL_SDA = D1;             //PIO5
       // CTRL_SCL = D2;             //PIO4
        break;  
    }  
  
};
