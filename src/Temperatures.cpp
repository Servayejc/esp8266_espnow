//2020-01-12//
#include "Temperatures.h"
#include <OneWire.h>
#include "global.h"
#include "arduino.h"
#include "control.h"
#include "utils.h"

Control CTRL;

#define ON 0
#define OFF 1
#define TH_ARRAY 3

//OneWire* oneWireBus = new OneWire[4];
//OneWire oneWireBus[] = {OneWire(D4), OneWire(D3)};  //ID 2

OneWire oneWireBus[] = {OneWire(4)}; // ID 2 - 4  

NewSensorsList NewSensors;
SensorsList Sensors;

float temperature;
//struct_message myData;
uint8_t relayStatus;
uint8_t relaySetPoint;

void StartConversion(uint8_t busNdx) {									
	#ifdef DEBUG_TEMPERATURES
		Serial.print(" Start conversion, Bus Index : ");
		Serial.println(busNdx);
	#endif
	oneWireBus[busNdx].reset();
	oneWireBus[busNdx].write(0xCC, 0);		 // SKIP ROM - Send command to all devices
	oneWireBus[busNdx].write(0x44, 0);      // START CONVERSION
}


void StartAllConversion() {
	uint8_t busCount = sizeof(oneWireBus)/sizeof(OneWire);
	
	for(uint8_t i = 0; i < busCount; ++i) {
		StartConversion(i);
	}
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
	float celcius = -50;
	#ifdef DEBUG_TEMPERATURES
		Serial.print("NDX = ");
		Serial.print(Ndx);
		Serial.print(" Bus Index : ");
		Serial.println(Sensors[Ndx].busNdx);
	#endif
	
	byte data[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	oneWireBus[Sensors[Ndx].busNdx].reset();    //Returns 1 if a device asserted a presence pulse, 0 otherwise. 
	oneWireBus[Sensors[Ndx].busNdx].select(Sensors[Ndx].Address);
	oneWireBus[Sensors[Ndx].busNdx].write(0xBE);						// READ SCRATCHPAD
		
	for ( byte i = 0; i < 9; i++) {         // we need 9 bytes
		data[i] = oneWireBus[Sensors[Ndx].busNdx].read();
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
	} else {
		celcius = -100;
	}

	#ifdef DEBUG_TEMPERATURES
		Serial.print("Temperature for ");
		Serial.print(Ndx);
		Serial.print(": ");
		Serial.println(celcius);
	#endif	
   
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
			float T = ReadTemp(i);
			if (T > -100) { // ignore invalid readings
				Sensors[i].TT = Sensors[i].TT + T;
				Sensors[i].NT ++;
			}
		#endif		
	}
	StartAllConversion();
}

String search(uint8_t oneWireNdx) {
  	
	String data = "";
	#ifdef DEBUG_SCAN_1WIRE
		Serial.print("Searching 1-wire bus on net ");
		Serial.println(oneWireNdx);
	#endif
  	
	uint8_t address[8]; 
	

	oneWireBus[oneWireNdx].reset();
	oneWireBus[oneWireNdx].reset_search();

	while (oneWireBus[oneWireNdx].search(address)) {
		Sensors.push_back(SensorEntry());
		int Ndx = Sensors.size()-1;
		Sensors[Ndx].Ndx = Ndx;
		memcpy(Sensors[Ndx].Address,address, 8); 
		Sensors[Ndx].busNdx = oneWireNdx;
		Sensors[Ndx].NT = 0;
		
		#ifdef DEBUG_SCAN_1WIRE
			Serial.print("+");
			Serial.print(Ndx);
			Serial.print(" Group ");
			Serial.print(oneWireNdx);
			Serial.print(" : ");
			for(byte i = 0; i < 8; ++i) {
				if (address[i] < 0x10) Serial.write('0');
				Serial.print(address[i], HEX);
				Serial.print(' ');
				data += String(address[i], HEX);
				data += " "; 
			}
			Serial.println();
		#endif
		Ndx ++;
		
		data += "\n";
		 
    }
	oneWireBus[oneWireNdx].reset_search();
	oneWireBus[oneWireNdx].reset();
	return(data);
}

void searchAll(){
	Sensors = {};
	search(0);
	// there is a bug in search procedure
	// when this procedure is called for the first time.
	if (Sensors.size() == 0){ 
		search(0);
	}
	uint8_t busCount = sizeof(oneWireBus)/sizeof(OneWire);	
	for(byte i = 1; i < busCount ; ++i) {
		search(i);
	}

}

void getReadings(uint8_t Ndx , struct_message *myData)
{
  if (Sensors.size() > Ndx) {
    temperature = ReadTemp(Ndx);
    memcpy(myData->deviceAddress, Sensors[Ndx].Address,8);
  } else {
    // simulation
    temperature = simulateTemps[Ndx];
    //temperature = 22.2;
  }
  myData->msgType = DATA;
  myData->deviceId = pairingData.deviceIds[Ndx];
  uint8_t deviceType = pairingData.deviceTypes[Ndx];
  myData->deviceType = deviceType;
  switch (deviceType)
  {
  case THERMOSTAT:
    if (temperature < RTCdata.temp_SP[Ndx] - 0.5)
    {
       RTCdata.curState[Ndx] = ON;
    }
    if (temperature > RTCdata.temp_SP[Ndx] + 0.5)
    {
       RTCdata.curState[Ndx] = OFF;
    }
 
    #ifdef DEBUG_CONTROL
      Serial.print(RTCdata.control[Ndx]);
      Serial.print("  :  ");
      Serial.println(RTCdata.curState[Ndx]); 
    #endif 
    
    CTRL.setChannel(RTCdata.control[Ndx]-1, (bool)RTCdata.curState[Ndx]);

    //CTRL.setAuto(true);
    myData->F1 = temperature;
    myData->U1 = (uint8_t)RTCdata.curState[Ndx];
    myData->F2 = RTCdata.temp_SP[Ndx];
    break;

  case THERMOMETER:
    myData->F1 = temperature;
    myData->U1 = (uint8_t)RTCdata.curState[Ndx];
    break;

  case RELAY:
    myData->F1 = -1;
    myData->F2 = -1;

    myData->U1 = relayStatus;
    myData->U2 = relaySetPoint;
    break;

    case TH_ARRAY:
    myData->F1 = temperature;
    myData->F2 = -1;
    myData->U1 = -1;
    myData->U2 = -1;
    break;
  }
  //esp_now_send(serverAddress, (uint8_t *)&myData, sizeof(myData));
  printData(*myData);
}

