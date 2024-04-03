/*
    ESPHelper.cpp
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


#include "ESPHelper.h"
#include <WiFiClientSecure.h>
#include "ESPHelperFS.h"



//Yes yes I know how messy it is to have all these different constructors that all call init AND
//have a bunch of overloaded init methods that do the same thing more or less but this lets me
//instantiate a blank ESPHelper globally and then in setup configure it without having to use new


/*
empty initializer wrapper
used for creating instances of ESPHelper that will be filled in later from a loaded config file

input: NA
output: NA
*/
ESPHelper::ESPHelper(){
	init("", "", "", "", "", 1883, "defaultWillTopic", "", 0, 1);
}


/*
initializer wrapper with single netInfo network

input: netInfo ptr
output: NA
*/
ESPHelper::ESPHelper(const netInfo *startingNet){
	init(startingNet->ssid, startingNet->pass, startingNet->mqttHost, startingNet->mqttUser, startingNet->mqttPass, startingNet->mqttPort, startingNet->willTopic, startingNet->willMessage, startingNet->willQoS, startingNet->willRetain);
}


/*
initializer wrapper with single network information and MQTT broker

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
output: NA
*/
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP){
	init(ssid, pass, mqttIP, "", "", 1883, "defaultWillTopic", "", 0, 1);
}


/*
initializer wrapper with single network information, MQTT broker and MQTT Last Will

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
	char ptr for MQTT last will topic
	char ptr for MQTT last will message
output: NA
*/
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *willTopic, const char *willMessage){
	init(ssid, pass, mqttIP, "", "", 1883, willTopic, willMessage, 0, 1);
}


/*
initializer wrapper with single network information, MQTT broker, MQTT Last Will and Testament options

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
	char ptr for MQTT last will topic
	char ptr for MQTT last will message
	int for MQTT will QOS
	int for MQTT will retain
output: NA
*/
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain){
	init(ssid, pass, mqttIP, "", "", 1883, willTopic, willMessage, willQoS, willRetain);
}


/*
initializer wrapper with single network information (MQTT user/pass)

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
	char ptr for MQTT username
	char ptr for MQTT password
	int for MQTT port
output: NA
*/
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort){
	init(ssid, pass, mqttIP, mqttUser, mqttPass, mqttPort, "defaultWillTopic", "", 0, 1);
}


/*
initializer wrapper with single network information (MQTT user/pass) + Testament

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
	char ptr for MQTT username
	char ptr for MQTT password
	int for MQTT port
	char ptr for MQTT last will topic
	char ptr for MQTT last will message
	int for MQTT will QOS
	int for MQTT will retain
output: NA
*/
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain){
	init(ssid, pass, mqttIP, mqttUser, mqttPass, mqttPort, willTopic, willMessage, willQoS, willRetain);
}


/*
initializer with single network information (no MQTT)

input: 
	char ptr for network SSID
	char ptr for network Password
output: NA
*/
ESPHelper::ESPHelper(const char *ssid, const char *pass){
	init(ssid, pass, "", "", "", 1883, "defaultWillTopic","",0,1);
}


/*
initializer wrapper with netInfo array and index

input: 
	netInfo ptr array
	uint8 count of networks available in the array
	uint8 index of the first network to use
output: NA
*/
ESPHelper::ESPHelper(netInfo *netList[], uint8_t netCount, uint8_t startIndex){
	_netList = netList;
	_netCount = netCount;
	_currentIndex = startIndex;

	//enable hopping since we have an array of networks to use
	_hoppingAllowed = true;

	//disable ota by default
	_useOTA = false;

	netInfo* tmp = netList[constrain(_currentIndex, 0, _netCount)];
	init(tmp->ssid, tmp->pass, tmp->mqttHost, tmp->mqttUser, tmp->mqttPass, tmp->mqttPort, tmp->willTopic, tmp->willMessage, tmp->willQoS, tmp->willRetain);
}

/*
initializer wrapper with a filename to be loaded from the filesystem

input: char ptr for filename to be loaded from the FS
output: NA
*/
ESPHelper::ESPHelper(const char* configFile){
	netInfo ESPConfig = loadConfigFile(configFile);
	init(ESPConfig.ssid, ESPConfig.pass, ESPConfig.mqttHost, ESPConfig.mqttUser, ESPConfig.mqttPass, ESPConfig.mqttPort, ESPConfig.willTopic, ESPConfig.willMessage, ESPConfig.willQoS, ESPConfig.willRetain);
}


/*
initialize the netinfo data and reset wifi. set hopping and OTA to off

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
	char ptr for MQTT username
	char ptr for MQTT password
	int for MQTT port
	char ptr for MQTT last will topic
	char ptr for MQTT last will message
	int for MQTT will QOS
	int for MQTT will retain
output: NA
*/
void ESPHelper::init(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain){
	//diconnect from and previous wifi networks
    WiFi.softAPdisconnect();
	WiFi.disconnect();

	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqttHost= mqttIP;
	_currentNet.mqttUser = mqttUser;
	_currentNet.mqttPass = mqttPass;
	_currentNet.mqttPort = mqttPort;
	_currentNet.willTopic = willTopic;
	_currentNet.willMessage = willMessage;
	_currentNet.willQoS = willQoS;
	_currentNet.willRetain = willRetain;


	//validate various bits of network/MQTT info
	validateConfig();
}



/*
checks which parts of a netInfo have been filled out
and updates internal flags noting which are set

input: NA
output: NA
*/
void ESPHelper::validateConfig(){
	//network pass
	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	//ssid
	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}

	//mqtt host
	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}

	//mqtt port
  	if(_currentNet.mqttPort == 0){_currentNet.mqttPort = 1883;}

  	//mqtt username
	if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	//mqtt password
	if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}

	//Will Topic
	if(_currentNet.willTopic[0] == '\0'){_willTopicSet = false;}
	else{_willTopicSet = true;}

	//Will Message
	if(_currentNet.willMessage[0] == '\0'){_willMessageSet = false;}
	else{_willMessageSet = true;}

}

/*
begin wrapper with a filename 
(used for blank initialized ESPHelper instances)

input: char ptr filename to be loaded for the FS
output: bool (true if SSID is set, false if not set)
*/
bool ESPHelper::begin(const char* filename){
	_currentNet = loadConfigFile(filename);
	bool returnVal = begin(_currentNet.ssid, _currentNet.pass, _currentNet.mqttHost, _currentNet.mqttUser, _currentNet.mqttPass, _currentNet.mqttPort, _currentNet.willTopic, _currentNet.willMessage, _currentNet.willQoS, _currentNet.willRetain);

	return returnVal;
}

/*
begin wrapper with a filename 
(used for blank initialized ESPHelper instances)

input: netInfo ptr
output: bool (true if SSID is set, false if not set)
*/
bool ESPHelper::begin(const netInfo *startingNet){
	return begin(startingNet->ssid, startingNet->pass, startingNet->mqttHost, startingNet->mqttUser, startingNet->mqttPass, startingNet->mqttPort, startingNet->willTopic, startingNet->willMessage, startingNet->willQoS, startingNet->willRetain);
}


/*
begin wrapper with single network information and MQTT broker
(used for blank initialized ESPHelper instances)

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
output: bool (true if SSID is set, false if not set)
*/
bool ESPHelper::begin(const char *ssid, const char *pass, const char *mqttIP){
	return begin(ssid, pass, mqttIP, "", "", 1883, "defaultWillTopic","",0,1);
}


/*
begin wrapper with single network information (no MQTT)
(used for blank initialized ESPHelper instances)

input: 
	char ptr for network SSID
	char ptr for network Password
output: bool (true if SSID is set, false if not set)
*/
bool ESPHelper::begin(const char *ssid, const char *pass){
	return begin(ssid, pass, "", "", "", 1883, "defaultWillTopic","",0,1);
}


/*
begin wrapper with single network information (MQTT user/pass) + Testament
(used for blank initialized ESPHelper instances)

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
	char ptr for MQTT username
	char ptr for MQTT password
	int for MQTT port
	char ptr for MQTT last will topic
	char ptr for MQTT last will message
	int for MQTT will QOS
	int for MQTT will retain
output: bool (true if SSID is set, false if not set)
*/
bool ESPHelper::begin(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort, const char *willTopic, const char *willMessage, const int willQoS, const int willRetain){
	init(ssid, pass, mqttIP, mqttUser, mqttPass, mqttPort, willTopic, willMessage, willQoS, willRetain);
	return begin();
}


/*
begin wrapper with single network information (MQTT user/pass)
(used for blank initialized ESPHelper instances)

input: 
	char ptr for network SSID
	char ptr for network Password
	char ptr for MQTT broker IP (or hostname)
	char ptr for MQTT username
	char ptr for MQTT password
	int for MQTT port
output: bool (true if SSID is set, false if not set)
*/
bool ESPHelper::begin(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort){
	return begin(ssid, pass, mqttIP, mqttUser, mqttPass, mqttPort, "defaultWillTopic","",0,1);
}


/*
start the wifi & mqtt systems and attempt connection (currently blocking)
return values from this function are somewhat legacy as data validation takes place elsewhere now

input: NA
output:
	true on: ssid set 
	false on: ssid not set
*/
bool ESPHelper::begin(){

	// Generate client name based on MAC address and last 8 bits of microsecond counter
	#ifdef ESP8266
	_clientName = "esp8266-";
	#else
	_clientName = "esp32-";
	#endif
	uint8_t mac[6];
	WiFi.macAddress(mac);
	_clientName += macToStr(mac);

	//set the wifi mode to station
	WiFi.mode(WIFI_STA);

	//as long as the SSID has been set, then try to connect to the network
	if(_ssidSet){

		if(_passSet){WiFi.begin(_currentNet.ssid, _currentNet.pass);}
		else{WiFi.begin(_currentNet.ssid);}
		WiFi.setAutoReconnect(true);
		WiFi.setSleep(false);
		

		//as long as an mqtt ip has been set create an instance of PubSub for client
		if(_mqttSet){
			client.setServer(_currentNet.mqttHost, _currentNet.mqttPort);

			//set the mqtt message callback if needed
			if(_mqttCallbackSet){client.setCallback(_mqttCallback);}
		}

		//define a dummy instance of mqtt so that it is instantiated if no mqtt ip is set
		else{
			client.setServer("192.0.2.0", _currentNet.mqttPort);
		}

		//set the mqtt client to use the secure client if available
		if(_useSecureClient){client.setClient(wifiClientSecure);}
		else{client.setClient(wifiClient);}


		//ota event handlers
		ArduinoOTA.onStart([]() {/* ota start code */});
		ArduinoOTA.onEnd([]() {
			//give the arduino a bit of time to finish up any remaining network activity
			delay(500);
			//on ota end we disconnect from wifi cleanly before restarting.
			WiFi.softAPdisconnect();
			WiFi.disconnect();
			int timeout = 0;
			//max timeout of 2seconds before just dropping out and restarting
			while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
				delay(10);
				timeout++;
			}
		});
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {/* ota progress code */});
		ArduinoOTA.onError([](ota_error_t error) {/* ota error code */});

		//initially attempt to connect to wifi when we begin (but only block for 2 seconds before timing out)
		int timeout = 0;	//counter for begin connection attempts
		while (((!client.connected() && _mqttSet) || WiFi.status() != WL_CONNECTED) && timeout < 200 ) {	//max 2 sec before timeout
			reconnect();
			delay(10);
			timeout++;
		}

		//attempt to start ota if needed
		OTA_begin();

		//mark the system as started and return
		_hasBegun = true;
		return true;
	}

	//if no ssid was set even then dont try to begin and return false
	return false;
}


/*
end the instance of ESPHelper (shutdown wifi, ota, mqtt)

input: NA
output: NA
*/
void ESPHelper::end(){
	ESPHelperFS::end();
	OTA_disable();
	client.disconnect();
	delay(20);
	WiFi.softAPdisconnect();
	WiFi.disconnect();

	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}

	_connectionStatus = NO_CONNECTION;

}


/*
attempts to load a config file from the filesystem - returns blank netInfo on failure
This will also create a new config with default values if none currently exists or is corrupted.

input: char ptr for a filename
output: netInfo struct loaded from FS
*/
netInfo ESPHelper::loadConfigFile(const char* filename){
	//initialize the loaded var to false until we load it from memory
	bool configLoaded = false;
	netInfo returnConfig;

	//create a static instance of the FS handler so that one isnt created by default but is left in memory if we
	//need to load a config
	static ESPHelperFS configLoader(filename);

	//attempt to load the config 3 times (really if it fails once it's probably going to fail again but why not try...)
	for(int tryCount = 0; tryCount < 3 && configLoaded == false; tryCount++){

		//attempt to start the filesystem
		if(configLoader.begin()){
			//attempt to load a configuration
	    	configLoaded = configLoader.loadNetworkConfig();

	    	//if no config loaded (either from corruption or no file), create a new config and close the FS
		    if(!configLoaded){
				debugPrintln("Could not load config - generating new config and restarting...");
				configLoader.createConfig(filename);
				configLoader.end();
				debugPrintln("Config File loading failed. Retrying...");
		    }

		    //if there is a good config, load it and close the FS
		    else{
		    	debugPrintln("Config loaded, getting info");
			    returnConfig = configLoader.getNetInfo();
			    configLoader.end();
			    debugPrintln("done.");
		    }
		}
	}

	//return either a loaded config or a blank one
	return returnConfig;
}


/*
attempts to saves a new config file with a given netInfo and filename

input: 
	netInfo to be saved
	char ptr for filename/path
output: 
	true on success
	false on failure
*/
bool ESPHelper::saveConfigFile(const netInfo config, const char* filename){
	
	//init ESPHelper FS and begin
	ESPHelperFS configLoader(filename);
	if(configLoader.begin()){
		configLoader.createConfig(&config);
		
		configLoader.end();
		return true;
	}

	//if the FS could not be started then return false (failed)
	return false;
}


/*
enables the use of a secure (SSL) connection to an MQTT broker.
(Make sure your mqtt port is set to one expecting a secure connection)

input: char ptr string for SSL fingerprint
output: NA
*/
void ESPHelper::useSecureClient(const char* fingerprint){
	_fingerprint = fingerprint;

	//fall back to wifi only connection if it was previously at full connection
	//(because we just changed how the device is going to connect to the mqtt broker)
	if(setConnectionStatus() == FULL_CONNECTION){
		_connectionStatus = WIFI_ONLY;
	}

	//if use of secure connection is set retroactivly (after begin), then disconnect and set the new client
	if(_hasBegun){
		client.disconnect();
		client.setClient(wifiClientSecure);
	}


	//flag use of secure client
	_useSecureClient = true;
}


/*
enables and sets up broadcast mode rather than station mode. This allows users to create a network from the ESP
and upload using OTA even if there is no network already present. This disables all MQTT connections

input: 
	char ptr for SSID to broadcast
	char ptr for network password
	IPAddress instance for the ESP IP

output: NA
*/
void ESPHelper::broadcastMode(const char* ssid, const char* password, const IPAddress ip){
	//disconnect from any previous wifi networks (max timeout of 2 seconds)
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}
	//set the mode for access point
	WiFi.mode(WIFI_AP);
	//config the AP
	WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
	//set the ssid and password
	WiFi.softAP(ssid, password);

	//run the wifi lost callback if we were previously connected to a network
	if(_wifiLostCallbackSet && _connectionStatus >= WIFI_ONLY){
		_wifiLostCallback();
	}

	//update the connection status
	_connectionStatus = BROADCAST;
	_broadcastIP = ip;
	strcpy(_broadcastSSID, ssid);
	strcpy(_broadcastPASS, password);



}


/*
disable broadcast mode and reset to station mode 
(causes a call to begin - may want to change this in the future...)

input: NA
output: NA
*/
void ESPHelper::disableBroadcast(){
	//disconnect from any previous wifi networks (max timeout of 2 seconds)
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}
	_connectionStatus = NO_CONNECTION;
	begin();
}


/*
main loop - should be called as often as possible - handles wifi/mqtt connection and mqtt handler

input: NA
output:
	true on: network/server connected
	false on: network or server disconnected
*/
int ESPHelper::loop(){
	if(_ssidSet){

		//check for good connections and attempt a reconnect if needed
		if (((_mqttSet && !client.connected()) || setConnectionStatus() < WIFI_ONLY) && _connectionStatus != BROADCAST) {
			reconnect();
		}

		//run the wifi loop as long as the connection status is at a minimum of BROADCAST
		if(_connectionStatus >= BROADCAST){

			//run the MQTT loop if we have a full connection
			if(_connectionStatus == FULL_CONNECTION){client.loop();}

			//run the heartbeat
			heartbeat();

			//check for whether we want to use OTA and whether the system is running
			if(_useOTA && _OTArunning) {ArduinoOTA.handle();}

			//if we want to use OTA but its not running yet, start it up.
			else if(_useOTA && !_OTArunning){
				OTA_begin();
				ArduinoOTA.handle();
			}


			return _connectionStatus;
		}

		yield();
	}

	//return -1 for no connection because of bad network info
	return -1;
}


/*
subscribe to a speicifc topic (does not add to topic list)

input:
	char ptr to a topic to subscribe to
	int for QOS of the subscription
output:
	true on: subscription success
	false on: subscription failed (either from PubSub lib or network is disconnected)
*/
bool ESPHelper::subscribe(const char* topic, int qos){
	if(_connectionStatus == FULL_CONNECTION){
		//set the return value to the output of subscribe
		bool returnVal = client.subscribe(topic, qos);
		//Serial.printf("Subscribe to: %s - %s\n", topic, returnVal == true ? "Success" : "Failure");
		//loop mqtt client
		client.loop();
		return returnVal;
	}

	//if not fully connected return false
	else{return false;}
}

/*
add a topic to the list of subscriptions and attempt to subscribe to the topic on the spot

input:
	char ptr for a topic to subscibe to
output:
	true on: subscription added to list (does not guarantee that the topic was subscribed to, only that it was added to the list)
	false on: subscription not added to list
*/
bool ESPHelper::addSubscription(const char* topic){
	//default return value is false
	bool subscribed = false;

	//loop throough finding the next available slot for a subscription and add it
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed == false){
			_subscriptions[i].topic = topic;
			_subscriptions[i].isUsed = true;
			subscribed = true;
			break;
		}
	}

	//if added to the list, subscibe to the topic
	if(subscribed){subscribe(topic, _qos);}

	return subscribed;
}


/*
loops through list of subscriptions and attempts to subscribe to all topics

input: NA
output: NA
*/
void ESPHelper::resubscribe(){
	debugPrintln("Resubscribing to all topics");
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			debugPrint("Topic: "); debugPrintln(_subscriptions[i].topic);
			subscribe(_subscriptions[i].topic, _qos);
			yield();
		}
	}
}


/*
attempts to remove a topic from the topic list

input:
	char ptr to a topic to subscribe to
output:
	true on: subscription removed from list (does not guarantee that the topic was unsubscribed from, only that it was removed from the list)
	false on: topic was not found in list and therefore cannot be removed
*/
bool ESPHelper::removeSubscription(const char* topic){
	bool returnVal = false;
	String topicStr = topic;

	//loop through all subscriptions
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		//if an element is used, check for it being the one we want to remove
		if(_subscriptions[i].isUsed){
			String subStr = _subscriptions[i].topic;
			if(subStr.equals(topicStr)){
				//reset the used flag to false
				_subscriptions[i].isUsed = false;

				//unsubscribe
				client.unsubscribe(_subscriptions[i].topic);
				returnVal = true;
				break;
			}
		}
	}

	return returnVal;
}


/*
manually unsubscribes from a topic (This is basically just a wrapper for the pubsubclient function)

input: 
	char ptr to topic to unsubscribe from
output
	true on: sucessful unsubscription
	false on: could not unsubscribe (generally b/c it was not subscribed anyways)

*/
bool ESPHelper::unsubscribe(const char* topic){
	return client.unsubscribe(topic);
}


/*
publish to a specified topic

input:
	char ptr to topic to publish to
	char ptr to the payload to be published
output: NA
*/
void ESPHelper::publish(const char* topic, const char* payload){
	publish(topic, payload, false);
}


/*
publish to a specified topic with a given retain level

input:
	char ptr to topic to publish to
	char ptr to the payload to be published
	bool whether the MQTT broker should retain the message
output: NA
*/
void ESPHelper::publish(const char* topic, const char* payload, bool retain){
	client.publish(topic, payload, retain);
}

boolean ESPHelper::publishJson(const char* topic, JsonDocument& doc, bool retain){
	client.beginPublish(topic, measureJsonPretty(doc), retain);
	BufferingPrint bufferedClient(client, 32);
	serializeJsonPretty(doc, bufferedClient);
	bufferedClient.flush();
	// &bufferedclient.flush();
	client.endPublish();
	return true;
}


/*
set the callback function for MQTT

input:
	function ptr that matches the MQTT callback function signature in pubsubclient
output: NA
*/
void ESPHelper::setMQTTCallback(MQTT_CALLBACK_SIGNATURE){
	_mqttCallback = callback;

	//only set the callback if using mqtt AND the system has already been started. Otherwise just save it for later
	if(_hasBegun && _mqttSet) {
		client.setCallback(_mqttCallback);
	}
	_mqttCallbackSet = true;
}


/*
legacy funtion - here for compatibility. Sets the callback function for MQTT (see function above)

input:
	function ptr that matches the MQTT callback function signature in pubsubclient
output: 
	true (always)
*/
bool ESPHelper::setCallback(MQTT_CALLBACK_SIGNATURE){
	setMQTTCallback(callback);
	return true;
}


/*
sets a custom function to run when connection to wifi is established

input:
	void function ptr with no params
output: NA
*/
void ESPHelper::setWifiCallback(void (*callback)()){
	_wifiCallback = callback;
	_wifiCallbackSet = true;
}


/*
sets a custom function to run when connection to wifi is lost

input:
	void function ptr with no params
output: NA
*/
void ESPHelper::setWifiLostCallback(void (*callback)()){
	_wifiLostCallback = callback;
	_wifiLostCallbackSet = true;
}


/*
attempts to connect to wifi & mqtt server if not connected

input: NA	
output: NA
*/
void ESPHelper::reconnect() {
	static int tryCount = 0;


	if(reconnectMetro.check() && _connectionStatus != BROADCAST && setConnectionStatus() != FULL_CONNECTION){
		debugPrintln("Attempting WiFi Connection...");
		//attempt to connect to the wifi if connection is lost
		if(WiFi.status() != WL_CONNECTED){
			_connectionStatus = NO_CONNECTION;

			#ifdef ESP32
			reconnect();
			#else
			//WiFi.reconnect();
			#endif

			//increment try count each time it cannot connect (this is used to determine when to hop to a new network)
			tryCount++;
			if(tryCount == 20){
				//change networks (if possible) when we have tried to connnect 20 times unsucessfully
				changeNetwork();
				tryCount = 0;
				return;
			}
		}

		// make sure we are connected to WIFI before attemping to reconnect to MQTT
		//----note---- maybe want to reset tryCount whenever we succeed at getting wifi connection?
		if(WiFi.status() == WL_CONNECTED){
			//if the wifi previously wasnt connected but now is, run the callback
			if(_connectionStatus < WIFI_ONLY && _wifiCallbackSet){
				_wifiCallback();
			}


			debugPrintln("\n---WIFI Connected!---");
			_connectionStatus = WIFI_ONLY;


			//attempt to connect to mqtt when we finally get connected to WiFi
			if(_mqttSet){

				static int timeout = 0;	//allow a max of 5 mqtt connection attempts before timing out
				if (!client.connected() && timeout < 5) {
					debugPrint("Attemping MQTT connection");

					
					client.disconnect();
					client.setServer(_currentNet.mqttHost, _currentNet.mqttPort);
					
					
					if(_useSecureClient){client.setClient(wifiClientSecure);}
					else{client.setClient(wifiClient);}


					int connected = 0;

					//connect to mqtt with user/pass
					if (_mqttUserSet && _willMessageSet) {
						debugPrintln(" - Using user & last will");
						debugPrintln(String("\t Client Name: " + String(_clientName.c_str())));
						debugPrintln(String("\t User Name: " + String(_currentNet.mqttUser)));
						debugPrintln(String("\t Password: " + String(_currentNet.mqttPass)));
						debugPrintln(String("\t Will Topic: " + String(_currentNet.willTopic)));
						debugPrintln(String("\t Will QOS: " + String(_currentNet.willQoS)));
						debugPrintln(String("\t Will Retain?: " + String(_currentNet.willRetain)));
						debugPrintln(String("\t Will Message: " + String(_currentNet.willMessage)));
						connected = client.connect((char*) _clientName.c_str(), _currentNet.mqttUser, _currentNet.mqttPass, _currentNet.willTopic, (int) _currentNet.willQoS, _currentNet.willRetain, (char*) _currentNet.willMessage);
					}

					//connect to mqtt without credentials
					else if (!_mqttUserSet && _willMessageSet) {
						debugPrintln(" - Using last will");
						debugPrintln(String("\t Client Name: " + String(_clientName.c_str())));
						debugPrintln(String("\t Will Topic: " + String(_currentNet.willTopic)));
						debugPrintln(String("\t Will QOS: " + String(_currentNet.willQoS)));
						debugPrintln(String("\t Will Retain?: " + String(_currentNet.willRetain)));
						debugPrintln(String("\t Will Message: " + String(_currentNet.willMessage)));
						connected = client.connect((char*) _clientName.c_str(), _currentNet.willTopic, (int) _currentNet.willQoS, _currentNet.willRetain, (char*) _currentNet.willMessage);
					} else if (_mqttUserSet && !_willMessageSet) {
						debugPrintln(" - Using user");
						debugPrintln(String("\t Client Name: " + String(_clientName.c_str())));
						debugPrintln(String("\t User Name: " + String(_currentNet.mqttUser)));
						debugPrintln(String("\t Password: " + String(_currentNet.mqttPass)));
						connected = client.connect((char*) _clientName.c_str(), _currentNet.mqttUser, _currentNet.mqttPass);
					} else {
						debugPrintln(" - Using default");
						debugPrintln(String("\t Client Name: " + String(_clientName.c_str())));
						connected = client.connect((char*) _clientName.c_str());
					}

					//if connected, subscribe to the topic(s) we want to be notified about
					if (connected) {
						debugPrintln(" -- Connected");

						#if ESP_SDK_VERSION_MAJOR > 2
						//if using https, verify the fingerprint of the server before setting full connection (return on fail)
						if(_useSecureClient){
							if (wifiClientSecure.verify(_fingerprint, _currentNet.mqttHost)) {
								debugPrintln("Certificate Matches - SUCESS");
							} else {
								debugPrintln("Certificate Doesn't Match - FAIL");
								return;
							}
						}
						#else
						if(_useSecureClient){debugPrintln("Certificate Not Supported on this SDK Version. Must use SDK 2.x.x");}
						#endif

						if(_mqttCallbackSet){
							debugPrintln("Setting MQTT callback");
							client.setCallback(_mqttCallback);
						}

						_connectionStatus = FULL_CONNECTION;
						resubscribe();
						timeout = 0;
					}
					else{
						debugPrintln(" -- Failed");
					}
					timeout++;

				}

				//if we still cant connect to mqtt after 10 attempts increment the try count
				if(timeout >= 5 && !client.connected()){
					timeout = 0;
					tryCount++;
					if(tryCount == 20){
						changeNetwork();
						tryCount = 0;
						return;
					}
				}
			}


		}

		//reset the reconnect metro
		reconnectMetro.reset();
	}
}


/*
internal function used to set _connectionStatus based on the WiFi & MQTT status

input: NA
output: NA
*/
int ESPHelper::setConnectionStatus(){

	//assume no connection
	int returnVal = NO_CONNECTION;

	//make sure were not in broadcast mode
	if(_connectionStatus != BROADCAST){

		//if connected to wifi set the mode to wifi only and run the callback if needed
		if(WiFi.status() == WL_CONNECTED){
			if(_connectionStatus < WIFI_ONLY && _wifiCallbackSet){	//if the wifi previously wasnt connected but now is, run the callback
				_wifiCallback();
			}
			returnVal = WIFI_ONLY;

			//if mqtt is connected as well then set the status to full connection
			if(client.connected()){
				returnVal = FULL_CONNECTION;
			}
		}

		//assuming above fails, then wifi is not connected.
		//if the wifi is not connected and the wifi lost callback has been set, then call it
		else if(_connectionStatus >= WIFI_ONLY && _wifiLostCallbackSet){
			_wifiLostCallback();
		}
	}


	else{
		returnVal = BROADCAST;
	}

	//set the connection status and return
	_connectionStatus = returnVal;
	return returnVal;
}


/*
changes the current network settings to the next listed network if network hopping is allowed

input: NA
output: NA
*/
void ESPHelper::changeNetwork(){

	//only attempt to change networks if hopping is allowed
	if(_hoppingAllowed){
		//change the index/reset to 0 if we've hit the last network setting
		_currentIndex++;
		if(_currentIndex >= _netCount){_currentIndex = 0;}

		//set the current netlist to the new network
		_currentNet = *_netList[_currentIndex];

		//verify various bits of network info

		//network password
		if(_currentNet.pass[0] == '\0'){_passSet = false;}
		else{_passSet = true;}

		//ssid
		if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
		else{_ssidSet = true;}

		//mqtt host
		if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
		else{_mqttSet = true;}

		//mqtt username
		if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
		else{_mqttUserSet = true;}

		//mqtt password
		if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
		else{_mqttPassSet = true;}

	        //Will Topic
	        if(_currentNet.willTopic[0] == '\0'){_willTopicSet = false;}
	        else{_willTopicSet = true;}

	        //Will Message
	        if(_currentNet.willTopic[0] == '\0'){_willMessageSet = false;}
	        else{_willMessageSet = true;}

		debugPrint("Trying next network: ");
		debugPrintln(_currentNet.ssid);

		//update the network connection
		updateNetwork();
	}
}


/*
input:
	
output:
*/
void ESPHelper::updateNetwork(){
	debugPrintln("\tDisconnecting from WiFi");
	WiFi.disconnect();
	debugPrintln("\tAttempting to begin on new network");
	
	//set the wifi mode
	WiFi.mode(WIFI_STA);

	//connect to the network
	if(_passSet && _ssidSet){WiFi.begin(_currentNet.ssid, _currentNet.pass);}
	else if(_ssidSet){WiFi.begin(_currentNet.ssid);}
	else{WiFi.begin("NO_SSID_SET");}
	
	WiFi.setSleep(false);
	//#ifdef ESP32
	WiFi.setAutoReconnect(true);
	//#endif

	debugPrintln("\tSetting new MQTT server");
	//setup the mqtt broker info
	if(_mqttSet){client.setServer(_currentNet.mqttHost, _currentNet.mqttPort);}
	else{client.setServer("192.0.2.0", 1883);}

	debugPrintln("\tDone - Ready for next reconnect attempt");
}


/*
generate unique MQTT name from MAC addr

input:
	uint8* (array) representing the ESP mac address 
	
output:
	string version of that mac address #
*/
String ESPHelper::macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;
}


/*
change the current network info to a new netInfo - does not automatically disconnect from current network if already connected

input:
	netInfo for the network to connect to
output: NA
*/
void ESPHelper::setNetInfo(netInfo newNetwork){
	_currentNet = newNetwork;
	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
	_mqttUserSet = true;
}


/*
change the current network info to a new *netInfo - does not automatically disconnect from current network if already connected

input:
	netInfo ptr for the network to connect to
output: NA
*/
void ESPHelper::setNetInfo(netInfo *newNetwork){
	_currentNet = *newNetwork;
	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
	_mqttUserSet = true;
}

/*
return the current netInfo state

input: NA
output:
	netInfo of the current network
*/
netInfo ESPHelper::getNetInfo(){
	return _currentNet;
}


/*
return the current SSID

input: NA
output:
	char ptr referencing the current SSID	
*/
const char* ESPHelper::getSSID(){
	if(_ssidSet && _connectionStatus != BROADCAST){return _currentNet.ssid;}
	else if(_connectionStatus == BROADCAST){return _broadcastSSID;}
	return "SSID NOT SET";
}


/*
set a new SSID - does not automatically disconnect from current network if already connected

input:
	char ptr to the SSID to connect to
output: NA
*/
void ESPHelper::setSSID(const char* ssid){
	_currentNet.ssid = ssid;
	_ssidSet = true;
}


/*
return the current network password

input: NA
output:
	char ptr to the current network pass
*/
const char* ESPHelper::getPASS(){
	if(_passSet && _connectionStatus != BROADCAST){return _currentNet.pass;}
	else if(_connectionStatus == BROADCAST){return _broadcastPASS;}
	return "PASS NOT SET";
}


/*
set a new network password - does not automatically disconnect from current network if already connected

input:
	char ptr to the new network pass to use
output: NA
*/
void ESPHelper::setPASS(const char* pass){
	_currentNet.pass = pass;
	_passSet = true;
}


/*
return the current MQTT server IP

input: NA
output:
	char ptr to the current MQTT IP or Hostname
*/
const char* ESPHelper::getMQTTIP(){
	if(_mqttSet){return _currentNet.mqttHost;}
	return "MQTT IP NOT SET";
}


/*
set a new MQTT server IP or Hostname - does not automatically disconnect from current network/server if already connected

input:
	char ptr to a new MQTT IP or Hostname str
output: NA
*/
void ESPHelper::setMQTTIP(const char* mqttIP){
	_currentNet.mqttHost= mqttIP;
	_mqttSet = true;
}


/*
set a new MQTT server IP - does not automatically disconnect from current network/server if already connected
Includes MQTT user and Pass

input:
	char ptr to a new MQTT IP or Hostname str
	char ptr to MQTT username
	char ptr to MQTT password
output: NA
*/
void ESPHelper::setMQTTIP(const char* mqttIP, const char* mqttUser, const char* mqttPass){
	_currentNet.mqttHost = mqttIP;
	_currentNet.mqttUser = mqttUser;
	_currentNet.mqttPass = mqttPass;
	_mqttSet = true;
	_mqttUserSet = true;
}


/*
set a new MQTT Will - does not automatically disconnect from current network/server if already connected

input:
	char ptr to MQTT will topic
	char ptr to MQTT will payload
output: NA
*/	
void ESPHelper::setWill(const char *willTopic, const char *willMessage){
	_currentNet.willTopic = willTopic;
	_currentNet.willMessage = willMessage;
	_willTopicSet = true;
	_willMessageSet = true;
}


/*
set a new MQTT Will - does not automatically disconnect from current network/server if already connected

input:
	char ptr to MQTT will topic
	char ptr to MQTT will payload
	int to MQTT will QOS
	int to MQTT will retain 
output: NA
*/
void ESPHelper::setWill(const char *willTopic, const char *willMessage, const int willQoS, const int willRetain){
	_currentNet.willTopic = willTopic;
	_currentNet.willMessage = willMessage;
	_currentNet.willQoS = willQoS;
	_currentNet.willRetain = willRetain;
	_willTopicSet = true;
	_willMessageSet = true;
}


/*
return the QOS level for mqtt

input: NA
output:
	int for current MQTT QOS
*/
int ESPHelper::getMQTTQOS(){
	return _qos;

}


/*
set the QOS level for mqtt

input:
	int for MQTT QOS level
output: NA
*/
void ESPHelper::setMQTTQOS(int qos){
	_qos = qos;
}


/*
return the local IP address of the ESP as a string

input: NA
output:
	String representing the current IP address of the ESP
*/
String ESPHelper::getIP(){
	if(_connectionStatus != BROADCAST){
		return WiFi.localIP().toString();
	}
	else{
		return _broadcastIP.toString();
	}

}


/*
return the local IP address of the ESP

input: NA
output:
 	IPAddress instance representing the current IP address of the ESP
*/
IPAddress ESPHelper::getIPAddress(){
	if(_connectionStatus != BROADCAST){
		return WiFi.localIP();
	}
	else{
		return _broadcastIP;
	}
}


/*
get the current connection status of ESPHelper

input: NA
output:
	int for current ESPHelper connection status(refer to connStatus enum is sharedData.h)
*/
int ESPHelper::getStatus(){
	return _connectionStatus;
}


/*
enable or disable hopping - generally set automatically by initializer

input:
	bool true to allow hopping false otherwise
output: NA
*/
void ESPHelper::setHopping(bool canHop){
	_hoppingAllowed = canHop;
}


/*
DEBUG ONLY - print the subscribed topics list to the serial line

input: NA
output: NA
*/
void ESPHelper::listSubscriptions(){
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			debugPrintln(_subscriptions[i].topic);
		}
	}
}


/*
enable the connection heartbeat on a given pin

input:
	int for the pin to pulse
output: NA
*/
void ESPHelper::enableHeartbeat(int16_t pin){
	#ifdef DEBUG
		if(pin == 1){_heartbeatEnabled = false;}
		else{
			_heartbeatEnabled = true;
			_ledPin = pin;
			pinMode(_ledPin, OUTPUT);
			digitalWrite(_ledPin, HIGH);
		}
	#else
		_heartbeatEnabled = true;
		_ledPin = pin;
		pinMode(_ledPin, OUTPUT);
		digitalWrite(_ledPin, HIGH);
	#endif
}


/*
disable the connection heartbeat

input: NA
output: NA
*/
void ESPHelper::disableHeartbeat(){
	_heartbeatEnabled = false;
}


/*
handles the heartbeat blinking to indicate network connection

input: NA
output: NA
*/
void ESPHelper::heartbeat(){
	static Metro heartbeatMetro = Metro(10);
	static int counter = 0;

	static bool ledState = true;

	if(heartbeatMetro.check() && _heartbeatEnabled){
		if(counter == 1){
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(10);
			ledState = !ledState;
		}
		else if(counter == 2){
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(300);
			ledState = !ledState;
		}
		else if(counter == 3){
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(10);
			ledState = !ledState;
		}
		else{
			digitalWrite(_ledPin, ledState);
			heartbeatMetro.interval(1000);
			ledState = !ledState;
			counter = 0;
		}
		counter++;
	}
}


/*
enable use of OTA updates

input: NA
output: NA
*/
void ESPHelper::OTA_enable(){
	_useOTA = true;
	OTA_begin();
}


/*
begin the OTA subsystem but with a check for connectivity and enabled use of OTA

input: NA
output: NA
*/
void ESPHelper::OTA_begin(){
	if(_connectionStatus >= BROADCAST && _useOTA){
		ArduinoOTA.begin();
		_OTArunning = true;
	}
}


/*
disable use of OTA updates

input: NA
output: NA
*/
void ESPHelper::OTA_disable(){
	_useOTA = false;
	_OTArunning = false;
}


/*
set a password for OTA updates

input:
	char ptr containing the OTA password to be used
output: NA
*/
void ESPHelper::OTA_setPassword(const char* pass){
	ArduinoOTA.setPassword(pass);
}


/*
set the hostname of the ESP for OTA uploads

input:
	char ptr containing the intended hostname
output: NA
*/
void ESPHelper::OTA_setHostname(const char* hostname){
	strcpy(_hostname, hostname);
	ArduinoOTA.setHostname(_hostname);
}


/*
set the hostname of the ESP for OTA uploads and append the ESPHelper version number

input:
	char ptr containing the intended hostname
output: NA
*/
void ESPHelper::OTA_setHostnameWithVersion(const char* hostname){
	strcpy(_hostname, hostname);
	strcat(_hostname, "----");
	strcat(_hostname, VERSION);

	ArduinoOTA.setHostname(_hostname);
}


/*
returns the current ESPHelper hostname

input: NA
output: 
	char ptr containing the ESP hostname
*/
char* ESPHelper::getHostname(){
	return _hostname;
}


/*
returns internal pubsubclient ptr (use with caution)

input: NA
output: 
	pubsubclient ptr
*/
PubSubClient* ESPHelper::getMQTTClient(){
	return &client;
}


/*
sets a new buffer size for mqtt messages in/out

input: int bytes of how large the buffer should be
output: 
	true: success
	false: failure
*/
bool ESPHelper::setMQTTBuffer(int size){
	#if PUB_SUB_VERSION >= 28
		return client.setBufferSize(size);
	#else
		return false;
	#endif
}
