#ifndef CONTROL_H_
#define CONTROL_H_

#include "Arduino.h"

/*
#define CONTROL  0x20
#define MCP23017_ADDR    0x21
*/

class Control  {      
  private:
    bool isPresent = false; 
	uint8_t output 	; 
	void writeRegister(uint8_t reg, uint8_t value);
	uint8_t readRegister(uint8_t reg);
  public: 
    Control();            
    void test();
	bool present();
	void reset();
	uint8_t getStatus();
	void setCtrlChannel(uint8_t channel, bool value);
	//void setOn(uint8_t channel);
	//void setOff(uint8_t channel);
	void setAuto(bool state);
	bool isAuto();
	void resetMCP017();
	uint8_t getLevel();
	void initLevel();
};

extern Control CTRL;



#endif