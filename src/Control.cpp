#include <Wire.h>
#include "Arduino.h"
#include "Control.h"

#define CONTROL  0x38

Control::Control() {
  Wire.begin(D1,D2); 
}

bool Control::present(){
	isPresent = false;
	Wire.available();
    Wire.beginTransmission(CONTROL);
	if (Wire.endTransmission() == 0){
		Serial.println("Control found");
		isPresent = true;
	} 
	return isPresent;
}

void Control::test() {
	output = 0;
	for ( byte i = 0; i < 6; i++) {
		bitSet(output, i);
		Wire.beginTransmission(CONTROL);
		Wire.write(output);
		Wire.endTransmission();
		delay(200);
		bitClear(output, i);
		delay(200);
		Wire.beginTransmission(CONTROL);
		Wire.write(output);
		Wire.endTransmission();
	}
}

void Control::reset() {
	output = 0;
	Wire.beginTransmission(CONTROL);
	Wire.write(output);
	Wire.endTransmission();
}

uint8_t Control::getStatus(){
	return output;
}

void Control::setChannel(uint8_t channel, bool value){
	Serial.print(channel);
    Serial.print(" : ");
    Serial.println(value);
	
	if (channel == 9){
      digitalWrite(D5, value);
    } else {
      if ((channel > -1) && (channel < 6)){
		if (!value){
			bitSet(output,channel);
		} else {
			bitClear(output,channel);
		}
		Wire.beginTransmission(CONTROL);
		Wire.write(output);
		Wire.endTransmission();
	  }
	}
}

void Control::setAuto(bool state){
	if (state){
		bitSet(output, 7);
	} else {
		bitClear(output, 7);
	}
}

bool Control::isAuto(){
	return (output >= 127);
}