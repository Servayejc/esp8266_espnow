//2020-01-12//
#include "Temperatures.h"
#include <OneWire.h>
#include "Common.h"
#include "arduino.h"
OneWire ds3(D3);	
OneWire ds4(D4);		

NewSensorsList NewSensors;
SensorsList Sensors;

void StartConversion() {									// tested
	ds3.reset();
	ds3.write(0xCC, 0);		 // SKIP ROM - Send command to all devices
	ds3.write(0x44, 0);      // START CONVERSION
	//ds3.write(0xB4, 0);	 // Start voltage conversion	
}

byte dsCRC8(const uint8_t *addr, uint8_t len)//begins from LS-bit of LS-byte (OneWire.h)
{
  uint8_t crc = 0;
  while (len--)
  {
    uint8_t inbyte = *addr++;
    for (uint8_t i = 8; i; i--)
    {
      uint8_t mix = (crc ^ inbyte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;
      inbyte >>= 1;
    }
  }
  return crc;
}

float ReadTemp(byte Ndx) {
	byte data[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	float celcius = -50;
	if (Sensors[Ndx].Port == 0) {
		ds3.reset();    //Returns 1 if a device asserted a presence pulse, 0 otherwise. 
		ds3.select(Sensors[Ndx].Address);
		ds3.write(0xBE);						// READ SCRATCHPAD
		
		for ( byte i = 0; i < 9; i++) {         // we need 9 bytes
			data[i] = ds3.read();
			#ifdef DEBUG_TEMPERATURES
			Serial.print(data[i],HEX);
			Serial.print(" ");
			#endif	
		}
		
		byte crc1 = dsCRC8(data, 8);  //calculate crc
		#ifdef DEBUG_CRC
			Serial.print(" = ");
			Serial.print("Received CRC = ");
			Serial.println(data[8], HEX);
			Serial.print("Calculated CRC = ");
			Serial.println(crc1, HEX);    
		#endif
  
		if (crc1 == data[8]) {//compare : calculate with received
			// convert the data to actual temperature
			int16_t raw = (((int16_t) data[1]) << 11) | (((int16_t) data[0]) << 3);
			celcius = float(raw) / 128;
			//Serial.print("Temp = ");
			//Serial.println(celcius);
		} else {
			celcius = -100;
		}
	
		#ifdef DEBUG_TEMPERATURES
			Serial.print(Sensors[Ndx].Desc);
			Serial.print(": ");
			Serial.println(celcius);
		#endif	
	}
	return celcius;	  
}

void ReadTemperatures() {
	for ( byte i = 0; i < Sensors.size(); i++) {
		#ifdef DEBUG_TEST_CONTROL
			// simulate temperatures for regulation tests
			int channel = Sensors[i].Channel -1 ;
			if (channel > -1 ){
				bool isBitSet = Output & (1 << channel);
				if (isBitSet) {
					Sensors[i].TT = (Sensors[i].TT + 0.1);  
				}
				else
				{ 
					Sensors[i].TT = (Sensors[i].TT - 0.1); 
				}
				Sensors[i].NT = 1;
			}
		#else
			//int port = Sensors[i].Port;
			//Serial.print("Port ");
			//Serial.println(port);
			float T = ReadTemp(i);
			if (T > -100) { // ignore invalid readings
				Sensors[i].TT = Sensors[i].TT + T;
				Sensors[i].NT ++;
			}
		#endif		
	}
	// prepare next measurement
	StartConversion();
}

String search() {
  	String data = "";
	bool Same = true;
	Serial.println("Searching 1-wire bus");
  	NewSensors.clear();
	byte address[8];
  	int Ndx = 0;
  	while (ds3.search(address)) {
		//bool IsNew = true;
		for ( byte i = 0; i < Sensors.size(); i++) {
			Same = true;
			if (Sensors[i].Port == 0) {
				for ( byte b = 0; b < 8; b++) {
					if (!(Sensors[i].Address[b] == address[b])){
						Same = false;
					}
				}
				if (Same) {
					//Serial.print (" Exists");
					//IsNew = false;
					break;
				} 
			}			
		}
		if (true) {
			// add to new list
			Sensors.push_back(SensorEntry());
			Sensors[Ndx].Ndx = Ndx;
			memcpy(Sensors[Ndx].Address,address, 8); 
			Sensors[Ndx].Channel = 0;
			Sensors[Ndx].NT = 0;
			Ndx ++;
			for(byte i = 0; i < 8; ++i) {
				if (address[i] < 0x10) Serial.write('0');
				Serial.print(address[i], HEX);
				Serial.print(' ');
				data += String(address[i], HEX);
				data += " "; 

			}
			Serial.println();
			data += "\n";
		}
		
    }
	return(data);
  	ds3.reset_search();
}