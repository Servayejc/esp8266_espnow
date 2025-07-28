#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include "Arduino.h"
#include "Control.h"
#include "global.h"

#define CONTROL  0x38
#define MCP  0x20

/*
Control::setChannel(uint8_t channel, bool value)
	Channel numbers from 0 to 6 are used by 
    home made thermostat control 

	Channel numbers from 100 to 110 controls directly esp8266 IO ports 
		100 > D0 .. 110 > D10

	Caution:
	- D3 and D4 are used to connect 1-wire thermometers.
	- D0 are used to wakeup from deep sleep.	
*/
Adafruit_MCP23X17 mcp;
uint8_t level = 0;

void ICACHE_RAM_ATTR ISR(){
	level =  mcp.getLastInterruptPin();
	mcp.clearInterrupts();
}

uint8_t CtoIO[11] = {16, 5, 4, 0, 2, 14, 12, 13, 15, 3, 1};

Control::Control() {}

void Control::init(){
	Wire.begin(D1,D2);	
	if (!mcp.begin_I2C()) {
		Serial.println("Error.");
	  	while (1);
	}
	//Pins 0-8 are in input_pullup mode for water detection
	mcp.setupInterrupts(true, false, LOW);
	for ( byte i = 0; i < 9; i++) {
		mcp.pinMode(i, INPUT_PULLUP);
		mcp.setupInterruptPin(i, LOW);
    }
	//Pins 9 to 15 in output mode for Heater control
	for ( byte i = 9; i < 16; i++) {
		mcp.pinMode(i, OUTPUT);
    }
	mcp.setupInterrupts(true, false, LOW);
	mcp.clearInterrupts();
}

uint8_t Control::getLevel(){
	return level;
}


bool Control::present(uint8_t address){
	uint8_t error = false;
	//Wire.available();
	
	Wire.beginTransmission(address);
    error = Wire.endTransmission();
	if (error == 0){
		Serial.print("I2C found ");
		Serial.println(address);
	} 
	return !error;
}

void Control::test() {
	output = 0;
	for ( byte i = 0; i < 4; i++) {
		bitSet(output, i);
		mcp.writeGPIOB(output<<1);
	//	Serial.println(output<<1);
		delay(200);
		bitClear(output, i);
		mcp.writeGPIOB(output<<1);
    //	Serial.println(output<<1);
		delay(200);
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
	#ifdef DEBUG_CONTROL
	//  Serial.println("SET CHANNEL");
    //  Serial.print(channel);
    //  Serial.print("  :  ");
    //  Serial.println(value); 
    #endif 

	
	if (channel >= 100){
		digitalWrite(CtoIO[channel-100], !value);
		#ifdef DEBUG_CONTROL
		//	Serial.print("D = ");
		//	Serial.print(CtoIO[channel-100]);
		//	Serial.println(value,BIN);
		#endif
	} else {
	    if ((channel > -1) && (channel < 6)){
			if (!value){
				bitSet(output,channel);
			} else {
				bitClear(output,channel);
			}
		//	Serial.print("output : ");
		//
			//Wire.beginTransmission(CONTROL);
			//Wire.write(output);
			//Wire.endTransmission();
			mcp.writeGPIOB(output);

			#ifdef DEBUG_CONTROL
		//	  Serial.print("Output = ");
		//	  Serial.println(output,BIN);
			#endif
		}	
	}
	//Serial.println("--- END SET Channel");
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

void Control::scanI2C(){
	byte error, address;
	int nDevices;
	
	Serial.println("Scanning...");	
	nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
	Wire.beginTransmission(address);
    error = Wire.endTransmission();
	Serial.println(address);
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
	  switch (address)
	  {
	  case CONTROL:
	    Serial.print("CONTROL = ");
		break;  
	  case MCP:
	    Serial.print("MCP = ");
		break;
	  }
      if (address<16)
        Serial.print("0");
	  Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

