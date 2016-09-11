/*    
    ESPHelper.h 
    Copyright (c) 2016 ItKindaWorks All right reserved.
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

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Metro.h"

#define MAX_SUBSCRIPTIONS 25	//feel free to change this if you need more subsciptions

// #define DEBUG




#ifdef DEBUG
	#define debugPrint(x) Serial.print(x) //debug on
	#define debugPrintln(x) Serial.println(x) //debug on
#else
	#define debugPrint(x) {;} //debug off
	#define debugPrintln(x) {;} //debug off
#endif

struct netInfo {
	char* name;
	char* mqtt;
	char* ssid;
	char* pass;
};
typedef struct netInfo netInfo;

struct subscription{
	bool isUsed = false;
	char* topic;
};
typedef struct subscription subscription;


class ESPHelper{

public:
	int16_t _hoppingAllowed = false;
	bool _connected = false;

	netInfo _currentNet;
	netInfo *_currentNetwork;

	Metro reconnectMetro = Metro(500);
	
	PubSubClient client;

	ESPHelper();
	ESPHelper(netInfo *startingNet);
	ESPHelper(netInfo **startingNet, uint8_t netCount, uint8_t startIndex);
	ESPHelper(char *ssid, char *pass, char *mqttIP);

	bool begin();
	void end();

	bool loop();

	bool subscribe(char* topic);
	bool addSubscription(char* topic);
	bool removeSubscription(char* topic);
	void publish(char* topic, char* payload);

	void setCallback(MQTT_CALLBACK_SIGNATURE);

	void reconnect();

	char* getSSID();
	void setSSID(char *ssid);

	char* getPASS();
	void setPASS(char *pass);

	char* getMQTTIP();
	void setMQTTIP(char *mqttIP);

	String getIP();

	void setNetInfo(netInfo newNetwork);
	void setNetInfo(netInfo *newNetwork);
	netInfo* getNetInfo();

	void setHopping(bool canHop); 

	void listSubscriptions();

	void enableHeartbeat(int16_t pin);
	void disableHeartbeat();
	void heartbeat();

private:
	WiFiClient wifiClient;

	uint8_t _netCount = 0;
	uint8_t _currentIndex = 0;

	bool _ssidSet = false;
	bool _passSet = false;
	bool _mqttSet = false;

	netInfo **_netList;

	int16_t _ledPin = 2;
	bool _heartbeatEnabled = false;

	subscription _subscriptions[MAX_SUBSCRIPTIONS];


	void changeNetwork();

	String macToStr(const uint8_t* mac);

	bool checkParams();

	void resubscribe();
};

#endif


