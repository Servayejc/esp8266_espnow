#ifndef CONTROL_H_
#define CONTROL_H_

#include "Arduino.h"


class Control  {      
  private:
    bool isPresent; 
	uint8_t output;
	uint8_t level; 
  public: 
    Control();
	void init();            
    void test();
	bool present(uint8_t address);
	void reset();
	uint8_t getStatus();
	void setChannel(uint8_t channel, bool value);
	void setAuto(bool state);
	bool isAuto();
	void scanI2C();
	uint8_t getLevel();
};

extern Control CTRL;



#endif