#ifndef TEMPERATURES_H_
#define TEMPERATURES_H_

#include <Arduino.h>
#include "global.h"

class Temps   {      
  private:
    bool isPresent; 
	uint8_t output; 
    void search(uint8_t oneWireNdx);
    byte dsCRC8(const uint8_t *addr, uint8_t len);
    float ReadTemp(byte Ndx);
    void StartConversion(uint8_t busNdx);
    void ReadTemperatures();
  public: 
    Temps();  
    void searchAll();     
    void StartAllConversion();
    void getReadings(uint8_t Ndx, struct_message *myData);
};

extern Temps TEMPS;



#endif