#include <Wire.h>
#include "Arduino.h"
#include "Control.h"
#include "PCF8574.h"
#include "global.h"



/*
Control::setChannel(uint8_t channel, bool value)
	Channel numbers from 0 to 6 are used by 
    home made thermostat control 

	Channel numbers from 100 to 110 controls directly esp8266 IO ports 
		100 > D0 .. 110 > D10

	Caution:
	- D0 are used to wakeup from deep sleep.
	- D1 and D2 are used for I2C communication with the control board.
	- D3 and D4 are used to connect 1-wire buses.
	- D5 is used to reset the control board.
	- D6 is used to set deep sleep mode.
	
*/

//#define DEBUG_CONTROL

bool keyPressed = false;




//MCP23017 register addresses
#define IODIR_A		0x00
#define IPOL_A		0x02
#define IOCON		0x0A
#define GPPU_A		0x0C
#define GPIO_A		0x12

Control::Control() {
  
}

//------ MCP23017 code ---------
void Control::resetMCP017(){
	pinMode(MCP23017_RST,OUTPUT);
	digitalWrite(MCP23017_RST, LOW);
	delay(100);
	digitalWrite(MCP23017_RST, HIGH);	
  	delay(100);
}

void Control::writeRegister(uint8_t reg, uint8_t value){
  	Wire.beginTransmission(MCP23017_ADDR);
	Wire.write(static_cast<uint8_t>(reg));
	Wire.write(value);
	Wire.endTransmission();
}

uint8_t Control::readRegister(uint8_t reg)
{
	Wire.beginTransmission(MCP23017_ADDR);
	Wire.write(static_cast<uint8_t>(reg));
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDR, 1); // read 1 byte
    return Wire.read();
}

void Control::initLevel(){
	writeRegister(IOCON, 0b00100000);  // bit 5 = 1-Sequential operation disabled, address pointer does not increment.
    //port mode
    writeRegister(IODIR_A, 0xFF); // direction 1-input 0-output
	writeRegister(GPPU_A, 0xFF);  // pullups 1-enabled 0-disabled;;
	writeRegister(IPOL_A, 0xFF);  // inverted 1-inverted 0-normal;
}

uint8_t Control::getLevel(){
    //return readRegister(GPIO_A);
	uint8_t value = readRegister(GPIO_A);
	//Serial.println(value, BIN);
	if (value == 0) return 0;
    uint8_t position = 32 - __builtin_clz(value); // Position of MSB
	//Serial.println(position);
	return position;
}

//------ end of MCP23017 code ---------

bool Control::present(){
	isPresent = false;
	Wire.available();
    Wire.beginTransmission(CONTROL_ADDR);
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
		Wire.beginTransmission(CONTROL_ADDR);
		Wire.write(output);
		Wire.endTransmission();
		delay(200);
		bitClear(output, i);
		delay(200);
		Wire.beginTransmission(CONTROL_ADDR);
		Wire.write(output);
		Wire.endTransmission();
	}
}

uint8_t Control::getStatus(){
	return output;
}


// mapping of control channel number to esp8266 IO port number
//                   D0, D1, D2, D3, D4, D5, D6, D7, D8, D9, D10
uint8_t CtoIO[11] = {16, 5,  4,  0,  2,  14, 12, 13, 15, 3,  1}; 

void Control::setCtrlChannel(uint8_t channel, bool value){
	//	#ifdef DEBUG_CONTROL
	Serial.println("Control channel : " + String(channel) + " value : " + String(value));		
	//	#endif
	if (channel >= 100){
		Serial.print("Control Channel: IO");
		Serial.println(channel-100);	 
		pinMode(channel-100, OUTPUT); 
		digitalWrite(channel-100, !value);

		Serial.println(value);
	} else {
	    Serial.print("Control Channel: ");
		Serial.println(channel);
		// control of home made thermostat control board
		if ((channel >= 0) && (channel < 6)){
		if (!value){
			bitSet(output,channel);
		} else {
			bitClear(output,channel);
		}
		Wire.beginTransmission(CONTROL_ADDR);
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
	return bitRead(output, 7);
}

