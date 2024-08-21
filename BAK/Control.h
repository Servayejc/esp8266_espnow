#ifndef CONTROL_H_
#define CONTROL_H_

#include "Arduino.h"


class Control  {      
  private:
    bool isPresent; 
	uint8_t output; 
  public: 
    Control();            
    void test();
	bool present();
	void reset();
	uint8_t getStatus();
	void setOn(uint8_t channel);
	void setOff(uint8_t channel);
	void setAuto(bool state);
	bool isAuto();
};

extern Control CTRL;



#endif