//fdfff
#ifndef COMMON_H_
#define COMMON_H_


//#include <Common.h>
//#include <vector>            
#include <Arduino.h>
//#include <Ethernet.h>

#define CONTROL  0x38		
#define DISPLAY_ADD  0x3A 



//Extensions
extern bool controlPresent;
extern bool displayPresent; 
extern bool sdCardPresent;
//Regulator
extern float SetPoints[6];	// default values, set by wifi 
extern float Hysteresis;	
extern byte Output;
extern bool Auto;

// NTP Server
extern char* ntp_URL;

// WIFI
extern char* wifi_SSID;
extern char* wifi_password;

// NoIP                   
extern char* _NOIP_Client;        
extern char* _NOIP_User;			
extern char* _NOIP_Pass;

// NEW code
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
	char Desc[16];
	uint8_t Address[8] ;   
	uint8_t Port;	
	uint8_t Channel;
	bool Ready;
	int Time[9] ;	//	H*60 + M
	int SP[9];	//  SP * 100 	
	int Mode;
	float TT;
	int NT;
	int NdxSP;
	bool fault; 
	bool Ala;
	float SOC;
	float Bat;
 };

typedef std::vector<SensorEntry> SensorsList;
extern SensorsList Sensors;

struct StationEntry {
	int Port;
	//IPAddress IP;
	String URL;
	bool Ready;
};

/*struct EventEntry {
	float date;
	uint8_t job;
};

typedef std::vector<EventEntry> EventsList;
extern EventList Events;*/


typedef std::vector<StationEntry> StationsList;
extern StationsList Stations;

struct ChannelEntry {
	int SensorNdx;
	int Time[9] ;	//	H*60 + M
	int SP[9];		//  SP * 100
};

struct  TNumber {
    int g_nNumber;
};

extern TNumber Num;

struct TConfig{
	String hydro;
	
	String mailServer;
	int mailPort;
	String mailUser;
	String mailPass;
	String mailFrom;
	String mailTo;
	
	String NoipURL;						// modified
	String NoIPUser;					// modified
	String NoIPPass;					// modified
	
	String HostName;					// modified
	
	String ntpUrl;						// modified
	
	String ftpUser;						// modified
	String ftpPass;						// modified
}; 

extern TConfig _Config;

/*typedef std::vector<AlarmEntry> AlarmsList;
extern AlarmsList Alarms;

struct AlarmEntry {
	int ID;
	time_t Time;
	int Status;
};*/

typedef std::vector<ChannelEntry> ChannelList;
extern ChannelList Channels;
extern bool SensorsOk;
extern int LastMin;
extern bool DayChanged;
extern bool TempReaded;
const int maxMQTTpackageSize = 512;
const int maxMQTTMessageHandlers = 1;
extern long connection;
extern int arrivedcount;
extern bool Sended;
extern bool ConfigChanged;
extern bool SensorsChanged;
extern int offset;
extern int8_t OffsetHQ;
extern bool HQEnabled;
extern bool canSend;

extern time_t gmt;
extern time_t local;

extern String hydro;
extern String mailUser;
extern String mailPass;
extern String mailFrom;
extern String mailTo;
extern String NoipURL;						// modified
extern String NoIPUser;						// modified
extern String NoIPPass;						// modified
extern String HostName;						// modified
extern String ntpUrl;						// modified
extern String ftpUser;						// modified
extern String ftpPass;						// modified



#endif
