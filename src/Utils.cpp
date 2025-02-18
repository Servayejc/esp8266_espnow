#include "Utils.h"
#include "global.h"
#include "EEPROM.h"

uint16_t validity = 0000;
 

void saveRTCdata(bool init){
    RTCdata.validity = validity;
    if (init) {  
        for (byte i = 0; i < 12; i++){
            RTCdata.temp_SP[i] = 20;
            RTCdata.curState[i] = 0;
        }
        RTCdata.WiFiChannel = 1;
    }
   
    EEPROM.put(0,RTCdata);
    EEPROM.commit();
}

void readRTCdata(){
  // TODO Read data from RTC Mem
  Serial.print("RTCdata size : "); 
  Serial.println(sizeof(RTCdata));
//ESP.rtcUserMemoryRead (16, (uint32_t*) &RTCdata , sizeof(RTCdata ));
  EEPROM.begin(512);
  EEPROM.get(0, RTCdata);
    Serial.print("Validity : " );
    Serial.println(RTCdata.validity);
 if (RTCdata.validity != validity) { 
    Serial.println("RTCdata not present"); 
    saveRTCdata(true);
    EEPROM.get(0, RTCdata); 
    //ESP.rtcUserMemoryRead (16, (uint32_t*) &RTCdata , sizeof(RTCdata ));
  }else{
    Serial.println("RTCdata present");
    Serial.print("Validity : " );
    Serial.println(RTCdata.validity);
    
    for (byte i = 0; i < 12; i++){
        Serial.print(" [ ");
        Serial.print(RTCdata.temp_SP[i]);
        Serial.print(", ");
        Serial.print(RTCdata.curState[i]);
        Serial.print(" ] ");
        Serial.print("-");
    }
    Serial.println();
  }  
}

void printSetpoint(uint8_t relayStatus){
    Serial.println("AFTER SETPOINT");
    Serial.print("SP : ");
    Serial.println(RTCdata.temp_SP[setpoints.deviceId]);
    Serial.print("STATUS : ");
    Serial.println(relayStatus);
}

void printData(struct_message myData)
{
  Serial.println("-------------------");
  Serial.print("msgType : ");
  Serial.println(myData.msgType);
  Serial.print("ID : ");
  Serial.println(myData.deviceId);
  Serial.print("DeviceType : ");
  Serial.println(myData.deviceType);
  
  Serial.print("Sensor Address : ");
  for ( byte i = 0; i < 8; i++) {
  Serial.print(myData.deviceAddress[i],HEX);
  Serial.print(" ");
  }
  Serial.println();

  Serial.print("F1 : ");
  Serial.println(myData.F1);
  Serial.print("F2 : ");
  Serial.println(myData.F2);
  Serial.print("U1 : ");
  Serial.println(myData.U1);
  Serial.print("U2 : ");
  Serial.println(myData.U1);
  Serial.println("-------------------");
}

void updateSimulation(){
  for (byte i = 0; i < 12; i++){
    simulateTemps[i] += 1;
    if (simulateTemps[i] > 20+i){
      simulateTemps[i] = 15+i;
    } 
  }  
}
