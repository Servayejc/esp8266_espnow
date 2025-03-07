#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <arduino.h>
#include <vector>

//#define DEBUG_CONTROL
//#define DEBUG_CRC
//#define DEBUG_TEMPERATURES
//#define DEBUG_SCAN_1WIRE
//#define DEBUG_INCOMING_DATA
//#define DEBUG_SIMULATE_TEMP
//#define DEBUG_PAIRING_DATA
//#define DEBUG_PAIRING

#define BOARD_ID 3        // see Struct.json in server/data
#define SERVER_TEST
#ifdef SERVER_TEST
    #define SERVER_ID 99  //test
    #define DEBUG_CONTROL
    #define DEBUG_CRC
    #define DEBUG_TEMPERATURES
    #define DEBUG_SCAN_1WIRE
    #define DEBUG_INCOMING_DATA
    #define DEBUG_SIMULATE_TEMP
    #define DEBUG_PAIRING_DATA
    #define DEBUG_PAIRING
  #else
    #define SERVER_ID 10  //production
  #endif

 // connect GPI0 12 (D6) to ground to set deepsleep mode


extern int chan;
extern int pingTime;
extern bool ledPair;
extern uint8_t control[12]; 
extern int curState[12];

extern uint8_t simulateTemps[12];

typedef struct struct_ping
{
  uint8_t msgType;
  int msgID;
} struct_ping;

typedef struct struct_reset
{
  uint8_t msgType;
  uint8_t id;
} struct_reset;

typedef struct struct_SP
{
  uint8_t msgType;
  uint8_t PeerID;
  uint8_t ID;
  float temp_sp;
} struct_SP;
typedef struct struct_message
{
  uint8_t msgType;  
  uint8_t deviceId;
  uint8_t deviceType;
  uint8_t U1; // status;
  uint8_t U2;
  uint8_t U3;
  uint8_t U4;
  uint8_t U5;
  float F1;   // temp;
  float F2;   // temp_sp;
  float F3;
  float F4;
  float F5;  
  uint8_t deviceAddress[8];  // used for DS18B20 addresses
  
} struct_message;

typedef struct struct_pairing
{ // new structure for pairing
  uint8_t msgType;
  uint8_t network;
  uint8_t id;
  uint8_t macAddr[6];
  uint8_t channel;
  uint8_t deviceIds[12];
  uint8_t deviceTypes[12]; 
  uint8_t controlNdx[12];
} struct_pairing;

typedef struct struct_dataRTC {
  uint16_t validity;
  uint8_t temp_SP[12] = {}; 
  uint8_t curState[12] = {}; 
  uint8_t control[12] = {}; 
  uint8_t WiFiChannel;
} struct_dataRTC; 

extern struct_dataRTC RTCdata;
extern struct_message incomingReadings;
extern struct_message outgoingSetpoints;
extern struct_pairing pairingData;
extern struct_message setpoints;
extern struct_ping pingData;

typedef struct PeerEntry {
	uint8_t Ndx;
	uint8_t Type;
  uint8_t MAC[6];
  uint8_t Channel;
  uint8_t Connected;
  unsigned long LastData; 
} PeerEntry;

typedef std::vector<PeerEntry> PeerList;
extern PeerList Peers;

// DS18B20
struct NewSensorEntry {
	int Ndx;
 	uint8_t Address[8];
};
typedef std::vector<NewSensorEntry> NewSensorsList;
extern NewSensorsList NewSensors;

struct SP {
	int Time;
	int Value; 
};

struct SensorEntry {
	int Ndx;
  uint8_t Address[8] ;   
  uint8_t busNdx;
	float TT;
	int NT;
 };

typedef std::vector<SensorEntry> SensorsList;
extern SensorsList Sensors;
enum PairingStatus {PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED, PAIR_PAUSED};
enum PeerType { THERMOSTAT, THERMOMETER, RELAY, };
enum MessageType { PAIRING, DATA, SETPOINTS, PING, RESET, };
extern PeerType peerType;
extern MessageType messageType;

#endif