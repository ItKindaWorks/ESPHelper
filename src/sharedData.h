/*
    sharedTypes.h
    Copyright (c) 2019 ItKindaWorks Inc All right reserved.
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



#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#define VERSION "1-7-10"


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
			mqttUser("defaultMqttUser"),
			mqttPass("defaultMqttPass"),
			mqttPort(1883),
			ssid(_ssid),
			pass(_pass),
			otaPassword("defaultOTA_PASS"),
			hostname("defaultHostname"),
			willTopic("defaultWillTopic"),
			willMessage(""),
			willQoS(1),
			willRetain(1) {}

  //mqtt host | ssid | network pass | willTopic | willMessage
  netInfo(const char* _mqttHost,
    const char* _ssid,
    const char* _pass,
    const char* _willTopic,
    const char* _willMessage) :
    mqttHost(_mqttHost),
	mqttUser("defaultMqttUser"),
    mqttPass("defaultMqttPass"),
	mqttPort(1883),
    ssid(_ssid),
    pass(_pass),
    otaPassword("defaultOTA_PASS"),
    hostname("defaultHostname"),
    willTopic(_willTopic),
    willMessage(_willMessage),
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
			otaPassword("defaultOtaPASS"),
			hostname("defaultHostname"),
			willTopic(_willTopic),
			willMessage(_willMessage),
			willQoS(1),
			willRetain(1) {}


	//mqtt host | mqtt user| mqtt pass| mqtt port| ssid | network pass | willTopic | willMessage | willRetain | willQoS
	netInfo(const char* _mqttHost,
			const char* _mqttUser,
			const char* _mqttPass,
			int _mqttPort,
			const char* _ssid,
			const char* _pass,
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
			otaPassword("defaultOtaPASS"),
			hostname("defaultHostname"),
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




#endif