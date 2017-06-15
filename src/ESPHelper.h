/*    
    ESPHelper.h 
    Copyright (c) 2016 ItKindaWorks Inc All right reserved.
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

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Metro.h"

#define MAX_SUBSCRIPTIONS 25	//feel free to change this if you need more subsciptions

#define VERSION "1-2-6"

#define DEFAULT_QOS 1;	//at least once - devices are garunteed to get a message.

// #define DEBUG

enum connStatus {NO_CONNECTION, BROADCAST, WIFI_ONLY, FULL_CONNECTION};


#ifdef DEBUG
	#define debugPrint(x) Serial.print(x) //debug on
	#define debugPrintln(x) Serial.println(x) //debug on
#else
	#define debugPrint(x) {;} //debug off
	#define debugPrintln(x) {;} //debug off
#endif

struct netInfo {

	const char* name;
	const char* mqttHost;
	const char* mqttUser;
	const char* mqttPass;
	const char* ssid;
	const char* pass;
};

typedef struct netInfo netInfo;

struct subscription{
	bool isUsed = false;
	const char* topic;
};
typedef struct subscription subscription;


class ESPHelper{

public:
	int16_t _hoppingAllowed = false;
	bool _fullyConnected = false;

	netInfo _currentNet;
	netInfo *_currentNetwork;

	
	PubSubClient client;

	ESPHelper();
	ESPHelper(netInfo *startingNet);
	ESPHelper(netInfo **startingNet, uint8_t netCount, uint8_t startIndex = 0);
	ESPHelper(const char *ssid, const char *pass, const char *mqttIP);

	ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass);

	bool begin();
	void end();

	void broadcastMode(const char* ssid, const char* password, const IPAddress ip);
	void disableBroadcast();

	int loop();

	bool subscribe(const char* topic, int qos);
	bool addSubscription(const char* topic);
	bool removeSubscription(const char* topic);

	void publish(const char* topic, const char* payload);
	void publish(const char* topic, const char* payload, bool retain);

	bool setCallback(MQTT_CALLBACK_SIGNATURE);

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

	String getIP();

	int getStatus();

	void setNetInfo(netInfo newNetwork);
	void setNetInfo(netInfo *newNetwork);
	netInfo* getNetInfo();

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

	Metro reconnectMetro = Metro(500);

	WiFiClient wifiClient;

	String _clientName;

	int _connectionStatus = NO_CONNECTION;

	uint8_t _netCount = 0;
	uint8_t _currentIndex = 0;

	bool _ssidSet = false;
	bool _passSet = false;
	bool _mqttSet = false;
	bool _mqttUserSet = false;
  bool _mqttPassSet = false;

	bool _useOTA = false;
	bool _OTArunning = false;

	bool _hasBegun = false;

	netInfo **_netList;

	int16_t _ledPin = 2;
	bool _heartbeatEnabled = false;

	subscription _subscriptions[MAX_SUBSCRIPTIONS];

	char _hostname[64];

	int _qos = DEFAULT_QOS;

	IPAddress _apIP = IPAddress(192, 168, 1, 1);

	void changeNetwork();

	String macToStr(const uint8_t* mac);

	bool checkParams();

	void resubscribe();

	int setConnectionStatus();
};

#endif


