/*    
    ESPHelper.h 
    Copyright (c) 2017 ItKindaWorks Inc All right reserved.
    github.com/ItKindaWorks

    This file is part of ESPHelper

    ESPHelper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESPHelper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ESPHelper.  If not, see <http://www.gnu.org/licenses/>.
*/
    


#ifndef ESP_HELPER_H
#define ESP_HELPER_H

#ifdef ESP8266
	#include <ESP8266WiFi.h>
	#include <ESP8266mDNS.h>
#endif

#ifdef ESP32
	#include <ESPmDNS.h>
	#include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
// #include "ESPHelperWebConfig.h"



#include "Metro.h"

#define VERSION "1-5-2"

// #define DEBUG

#ifdef DEBUG
	#define debugPrint(x) Serial.print(x) //debug on
	#define debugPrintln(x) Serial.println(x) //debug on
#else
	#define debugPrint(x) {;} //debug off
	#define debugPrintln(x) {;} //debug off
#endif

//Maximum number of subscriptions that can be auto-subscribed
//feel free to change this if you need more subsciptions
#define MAX_SUBSCRIPTIONS 25	

#define DEFAULT_QOS 1;	//at least once - devices are guarantee to get a message.


enum connStatus {NO_CONNECTION, BROADCAST, WIFI_ONLY, FULL_CONNECTION};

struct netInfo {
	const char* name;
	const char* mqttHost;
	const char* mqttUser;
	const char* mqttPass;
	int mqttPort;
	const char* ssid;
	const char* pass;
	const char* otaPassword;
	const char* hostname;
	const char* willTopic;
	const char* willMessage;
	int willQoS;
	int willRetain;
	
	netInfo() : mqttPort(1883) {}

	//name | mqtt host | ssid | network pass
	netInfo(const char* _name,
			const char* _mqttHost,  
			const char* _ssid,
			const char* _pass) : 
			name(_name),
			mqttHost(_mqttHost), 
			ssid(_ssid),
			pass(_pass),
			mqttPort(1883),
			mqttUser("defaultMqttUser"),
			mqttPass("defaultMqttPass"),
			otaPassword("defaultOTA_PASS"),
			hostname("defaultHostname"),
			willTopic("defaultWillTopic"),
			willMessage(""),
			willQoS(1),
			willRetain(1) {}

	//mqtt host | mqtt user| mqtt pass| mqtt port| ssid | network pass
	netInfo(const char* _mqttHost, 
			const char* _mqttUser, 
			const char* _mqttPass, 
			int _mqttPort, 
			const char* _ssid,
			const char* _pass) : 
			mqttHost(_mqttHost), 
			mqttUser(_mqttUser),
			mqttPass(_mqttPass),
			mqttPort(_mqttPort),
			ssid(_ssid),
			pass(_pass),
			otaPassword("defaultOtaPASS"),
			hostname("defaultHostname"),
			willTopic("defaultWillTopic"),
			willMessage(""),
			willQoS(1),
			willRetain(1)  {}


	//mqtt host | mqtt user| mqtt pass| mqtt port| ssid | network pass | ota pass | hostname
	netInfo(const char* _mqttHost, 
			const char* _mqttUser, 
			const char* _mqttPass, 
			int _mqttPort, 
			const char* _ssid,
			const char* _pass,
			const char* _otaPassword,
			const char* _hostname) : 
			mqttHost(_mqttHost), 
			mqttUser(_mqttUser),
			mqttPass(_mqttPass),
			mqttPort(_mqttPort),
			ssid(_ssid),
			pass(_pass),
			otaPassword(_otaPassword),
			hostname(_hostname),
			willTopic("defaultWillTopic"),
			willMessage(""),
			willQoS(1),
			willRetain(1) {}


	//mqtt host | mqtt user| mqtt pass| mqtt port| ssid | network pass | ota pass | hostname | willTopic | willMessage
	netInfo(const char* _mqttHost, 
			const char* _mqttUser, 
			const char* _mqttPass, 
			int _mqttPort, 
			const char* _ssid,
			const char* _pass,
			const char* _otaPassword,
			const char* _hostname,
			const char* _willTopic,
			const char* _willMessage) : 
			mqttHost(_mqttHost), 
			mqttUser(_mqttUser),
			mqttPass(_mqttPass),
			mqttPort(_mqttPort),
			ssid(_ssid),
			pass(_pass),
			otaPassword(_otaPassword),
			hostname(_hostname),
			willTopic(_willTopic),
			willMessage(_willMessage),
			willQoS(1),
			willRetain(1) {}


	//mqtt host | mqtt user| mqtt pass| mqtt port| ssid | network pass | ota pass | hostname | willTopic | willMessage | willRetain | willQoS
	netInfo(const char* _mqttHost, 
			const char* _mqttUser, 
			const char* _mqttPass, 
			int _mqttPort, 
			const char* _ssid,
			const char* _pass,
			const char* _otaPassword,
			const char* _hostname,
			const char* _willTopic,
			const char* _willMessage,
			int _willQoS,
			int _willRetain) : 
			mqttHost(_mqttHost), 
			mqttUser(_mqttUser),
			mqttPass(_mqttPass),
			mqttPort(_mqttPort),
			ssid(_ssid),
			pass(_pass),
			otaPassword(_otaPassword),
			hostname(_hostname),
			willTopic(_willTopic),
			willMessage(_willMessage),
			willQoS(_willQoS),
			willRetain(_willRetain)  {}

};
// typedef struct netInfo netInfo;


struct subscription{
	bool isUsed = false;
	const char* topic;
};
typedef struct subscription subscription;






class ESPHelper{

public:	
	ESPHelper();
	ESPHelper(const netInfo *startingNet);
	ESPHelper(netInfo *netList[], uint8_t netCount, uint8_t startIndex = 0);	//remember netInfo *netList[] is really netinfo**
	ESPHelper(const char *ssid, const char *pass, const char *mqttIP);
	ESPHelper(const char *ssid, const char *pass);
	ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort);
	ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *willTopic, const char *willMessage);
	ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain);
	ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain);

	ESPHelper(const char* configFile);

	
	bool begin(const char* filename);
	bool begin(const netInfo *startingNet);
	bool begin(const char *ssid, const char *pass, const char *mqttIP);
	bool begin(const char *ssid, const char *pass);
	bool begin(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort);
	bool begin(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort, const char *willTopic, const char *willMessage);
	bool begin(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain);
	bool begin();
	void end();

	netInfo loadConfigFile(const char* filename);
	bool saveConfigFile(const netInfo config, const char* filename);

	void useSecureClient(const char* fingerprint);

	void broadcastMode(const char* ssid, const char* password, const IPAddress ip);
	void disableBroadcast();

	int loop();

	bool subscribe(const char* topic, int qos);
	bool addSubscription(const char* topic);
	bool removeSubscription(const char* topic);
	bool unsubscribe(const char* topic);

	void publish(const char* topic, const char* payload);
	void publish(const char* topic, const char* payload, bool retain);

	bool setCallback(MQTT_CALLBACK_SIGNATURE);
	void setMQTTCallback(MQTT_CALLBACK_SIGNATURE);

	void setWifiCallback(void (*callback)());

	void reconnect();

	void updateNetwork();	//manually disconnect and reconnecting to network/mqtt using current values (generally called after setting new network values)

	const char* getSSID();
	void setSSID(const char *ssid);

	const char* getPASS();
	void setPASS(const char *pass);

	const char* getMQTTIP();
	void setMQTTIP(const char *mqttIP);
	void setMQTTIP(const char *mqttIP, const char *mqttUser, const char *mqttPass);

	int getMQTTQOS();
	void setMQTTQOS(int qos);

	void setWill(const char *willTopic, const char *willMessage);
	void setWill(const char *willTopic, const char *willMessage, const int willQoS, const int willRetain);

	String getIP();
	IPAddress getIPAddress();

	int getStatus();

	void setNetInfo(netInfo newNetwork);
	void setNetInfo(netInfo *newNetwork);
	// netInfo* getNetInfo();
	netInfo getNetInfo();

	void setHopping(bool canHop); 

	void listSubscriptions();

	void enableHeartbeat(int16_t pin);
	void disableHeartbeat();
	void heartbeat();

	void OTA_enable();
	void OTA_disable();
	void OTA_begin();
	void OTA_setPassword(const char* pass);
	void OTA_setHostname(const char* hostname);
	void OTA_setHostnameWithVersion(const char* hostname);

private:

	void init(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain);
	void validateConfig();

	void changeNetwork();

	String macToStr(const uint8_t* mac);

	bool checkParams();

	void resubscribe();

	int setConnectionStatus();

	netInfo _currentNet;
	
	PubSubClient client;

	Metro reconnectMetro = Metro(500);

	WiFiClient wifiClient;
	WiFiClientSecure wifiClientSecure;
	const char* _fingerprint;
	bool _useSecureClient = false;


	String _clientName;

	void (*_wifiCallback)();
	bool _wifiCallbackSet = false;

	#ifdef ESP8266
		std::function<void(char*, uint8_t*, unsigned int)> _mqttCallback;
	#endif
	#ifdef ESP32
		void(*_mqttCallback)(char*, uint8_t*, unsigned int) ;
	#endif
	bool _mqttCallbackSet = false;

	int _connectionStatus = NO_CONNECTION;

	uint8_t _netCount = 0;
	uint8_t _currentIndex = 0;

	bool _ssidSet = false;
	bool _passSet = false;
	bool _mqttSet = false;
	bool _mqttUserSet = false;
  	bool _mqttPassSet = false;
	bool _willTopicSet = false;
  	bool _willMessageSet = false;

	bool _useOTA = false;
	bool _OTArunning = false;

	bool _hoppingAllowed = false;

	bool _hasBegun = false;

	netInfo **_netList;

	int16_t _ledPin = 2;
	bool _heartbeatEnabled = false;

	subscription _subscriptions[MAX_SUBSCRIPTIONS];

	char _hostname[64];

	int _qos = DEFAULT_QOS;

	IPAddress _apIP = IPAddress(192, 168, 1, 1);

	
};

#endif


