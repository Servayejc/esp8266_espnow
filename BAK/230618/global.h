#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <arduino.h>
//#include <esp_wifi.h>

#include <vector>

#define SERVER_ID 10

extern int chan;
extern int pingTime;

enum PairingStatus {PAIR_REQUEST, PAIR_REQUESTED, PAIR_PAIRED, };
enum PeerType { THERMOSTAT, THERMOMETER, RELAY, };
enum MessageType { PAIRING, DATA, SETPOINTS, PING, };


extern PeerType peerType;
extern MessageType messageType;


typedef struct struct_ping
{
  uint8_t msgType;
  int msgID;
} struct_ping;


/*typedef struct struct_SP
{
  uint8_t msgType;
  uint8_t PeerID;
  uint8_t ID;
  float temp_sp;
} struct_SP;*/

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
  unsigned int readingId;
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
} struct_pairing;

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


#endif