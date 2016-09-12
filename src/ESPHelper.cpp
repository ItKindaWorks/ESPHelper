/*    
    ESPHelper.cpp
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


#include "ESPHelper.h"

//empy initializer 
ESPHelper::ESPHelper(){	}

//initializer with single netInfo network
ESPHelper::ESPHelper(netInfo *startingNet){		
	_currentNet = *startingNet;

	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;

	_hoppingAllowed = false;

	_useOTA = false;
}


//initializer with netInfo array and index
ESPHelper::ESPHelper(netInfo *netList[], uint8_t netCount, uint8_t startIndex){	
	_netList = netList;
	_netCount = netCount;
	_currentIndex = startIndex;

	_hoppingAllowed = true;

	_useOTA = false;

	_currentNet = *netList[constrain(_currentIndex, 0, _netCount)];

	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
}

//initializer with single network information
ESPHelper::ESPHelper(char *ssid, char *pass, char *mqttIP){	
	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqtt = mqttIP;

	_hoppingAllowed = false;

	_useOTA = false;

	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
}

//start the wifi & mqtt systems and attempt connection (currently blocking)
	//true on: parameter check validated & network/mqtt connection
	//false on: parameter check failed
bool ESPHelper::begin(){	

	if(checkParams()){

		client = PubSubClient(_currentNet.mqtt, 1883, wifiClient);

		WiFi.mode(WIFI_STA);
		WiFi.begin(_currentNet.ssid, _currentNet.pass);

		ArduinoOTA.onStart([]() {/* ota start code */});
		ArduinoOTA.onEnd([]() {/* ota end code */});
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {/* ota progress code */});
		ArduinoOTA.onError([](ota_error_t error) {/* ota error code */});


		while (!client.connected() || WiFi.status() != WL_CONNECTED ) {
			reconnect();
			delay(10);
		}

		OTA_begin();
		
		_hasBegun = true;
		return true;
	}
	return false;
}

void ESPHelper::end(){
	OTA_disable();
	WiFi.disconnect();
}

//main loop - should be called as often as possible - handles wifi/mqtt connection and mqtt handler
	//true on: network/server connected
	//false on: network or server disconnected
bool ESPHelper::loop(){	
	if(checkParams()){
		if (!client.connected() || WiFi.status() != WL_CONNECTED) {
			reconnect();
			_connected = false;
			return false;
		}
		else{
			client.loop();
			heartbeat();

			//check for whether we want to use OTA and whether the system is running
			if(_useOTA && _OTArunning) {ArduinoOTA.handle();}

			//if we want to use OTA but its not running yet, start it up.
			else if(_useOTA && !_OTArunning){
				OTA_begin();
				ArduinoOTA.handle();
			}

			_connected = true;
			return true;
		}
	}
}

//subscribe to a speicifc topic (does not add to topic list)
	//true on: subscription success
	//false on: subscription failed (either from PubSub lib or network is disconnected)
bool ESPHelper::subscribe(char* topic){		
	if(_connected){return client.subscribe(topic);}
	else{return false;}
}

//add a topic to the list of subscriptions and attempt to subscribe to the topic on the spot
	//true on: subscription added to list (does not guarantee that the topic was subscribed to, only that it was added to the list)
	//false on: subscription not added to list
bool ESPHelper::addSubscription(char* topic){	
	bool subscribed = false;
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed == false){
			_subscriptions[i].topic = topic;
			_subscriptions[i].isUsed = true;
			subscribed = true;
			break;
		}
	}
	if(subscribed){subscribe(topic);}
	

	return subscribed;
}

//loops through list of subscriptions and attempts to subscribe to all topics
void ESPHelper::resubscribe(){	
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			subscribe(_subscriptions[i].topic);
		}
	}
}

//attempts to remove a topic from the topic list
	//true on: subscription removed from list (does not guarantee that the topic was unsubscribed from, only that it was removed from the list)
	//false on: topic was not found in list and therefore cannot be removed
bool ESPHelper::removeSubscription(char* topic){	
	bool returnVal = false;
	String topicStr = topic;
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			String subStr = _subscriptions[i].topic;
			if(subStr.equals(topicStr)){
				_subscriptions[i].isUsed = false;
				client.unsubscribe(_subscriptions[i].topic);
				returnVal = true;
				break;
			}
		}
	}

	return returnVal;
}

//publish to a specified topic
void ESPHelper::publish(char* topic, char* payload){		
	client.publish(topic, payload);
}

//set the callback function for MQTT
	//true on: mqtt has been initialized
	//false on: mqtt not been inistialized
bool ESPHelper::setCallback(MQTT_CALLBACK_SIGNATURE){	
	if(_hasBegun) {
		client.setCallback(callback);
		return true;
	}
	else{
		return false;
	}
}

//attempts to connect to wifi & mqtt server if not connected
void ESPHelper::reconnect() {		
	static int tryCount = 0;

	if(reconnectMetro.check()){
		//attempt to connect to the wifi if connection is lost
		if(WiFi.status() != WL_CONNECTED){
			_connected = false;
			debugPrint(".");
			tryCount++;
			if(tryCount == 20){
				changeNetwork();
				tryCount = 0;
				return;
			}
		}

		// make sure we are connected to WIFI before attemping to reconnect to MQTT
		if(WiFi.status() == WL_CONNECTED){
			debugPrintln("\n---WIFI Connected!---");
		
			while (!client.connected()) {
				debugPrint("Attemping MQTT connection");

				// Generate client name based on MAC address and last 8 bits of microsecond counter
				String clientName;
				clientName += "esp8266-";
				uint8_t mac[6];
				WiFi.macAddress(mac);
				clientName += macToStr(mac);

				//if connected, subscribe to the topic(s) we want to be notified about
				if (client.connect((char*) clientName.c_str())) {
					debugPrintln(" -- Connected");
					_connected = true;
					resubscribe();
				}
				else{
					_connected = false;
					debugPrintln(" -- Failed");
				}
			}
		}

		reconnectMetro.reset();
	}
}

//changes the current network settings to the next listed network if network hopping is allowed
void ESPHelper::changeNetwork(){	

	if(_hoppingAllowed){
		_currentIndex++;
		if(_currentIndex >= _netCount){_currentIndex = 0;}

		_currentNet = *_netList[_currentIndex];
	}

	debugPrint("Trying next network: ");
	debugPrintln(_currentNet.ssid);

	debugPrintln("\tDisconnecting from WiFi");
	WiFi.disconnect();
	debugPrintln("\tAttempting to begin on new network");
	WiFi.begin(_currentNet.ssid, _currentNet.pass);
	debugPrintln("\tSetting new MQTT server");
	client.setServer(_currentNet.mqtt, 1883);
	debugPrintln("\tDone - Ready for next reconnect attempt");
}

void ESPHelper::updateNetwork(){
	debugPrintln("\tDisconnecting from WiFi");
	WiFi.disconnect();
	debugPrintln("\tAttempting to begin on new network");
	WiFi.begin(_currentNet.ssid, _currentNet.pass);
	debugPrintln("\tSetting new MQTT server");
	client.setServer(_currentNet.mqtt, 1883);
	debugPrintln("\tDone - Ready for next reconnect attempt");
}

//generate unique MQTT name from MAC addr
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

//change the current network info to a new netInfo - does not automatically disconnect from current network if already connected
void ESPHelper::setNetInfo(netInfo newNetwork){	
	_currentNet = newNetwork;
	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
}

//change the current network info to a new *netInfo - does not automatically disconnect from current network if already connected
void ESPHelper::setNetInfo(netInfo *newNetwork){ 	
	_currentNet = *newNetwork;
	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
}

//return the current netInfo state
netInfo* ESPHelper::getNetInfo(){	
	return &_currentNet;
}

//return the current SSID
char* ESPHelper::getSSID(){			
	if(_ssidSet){return _currentNet.ssid;}
	return "SSID NOT SET";
}
//set a new SSID - does not automatically disconnect from current network if already connected
void ESPHelper::setSSID(char* ssid){		
	_currentNet.ssid = ssid;
	_ssidSet = true;
}

//return the current network password
char* ESPHelper::getPASS(){			
	if(_passSet){return _currentNet.pass;}
	return "PASS NOT SET";
}
//set a new network password - does not automatically disconnect from current network if already connected
void ESPHelper::setPASS(char* pass){ 	
	_currentNet.pass = pass;
	_passSet = true;
}

//return the current MQTT server IP
char* ESPHelper::getMQTTIP(){		
	if(_mqttSet){return _currentNet.mqtt;}
	return "MQTT IP NOT SET";
}
//set a new MQTT server IP - does not automatically disconnect from current network/server if already connected
void ESPHelper::setMQTTIP(char* mqttIP){ 
	_currentNet.mqtt = mqttIP;
	_mqttSet = true;
}


String ESPHelper::getIP(){
	return WiFi.localIP().toString();
}




//make sure all network parameters are set
	//true on: all network parameters have been set 
	//false on: not all network parameters have been set 
bool ESPHelper::checkParams(){		
	if(_ssidSet && _passSet && _mqttSet){return true;}
	return false;
}

//enable or disable hopping - generally set automatically by initializer
void ESPHelper::setHopping(bool canHop){	
	_hoppingAllowed = canHop;
}

//DEBUG ONLY - print the subscribed topics list to the serial line
void ESPHelper::listSubscriptions(){
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			debugPrintln(_subscriptions[i].topic);
		}
	}
}



//enable the connection heartbeat on a given pin
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

//disable the connection heartbeat
void ESPHelper::disableHeartbeat(){			
	_heartbeatEnabled = false;
}

//heartbeat to indicate network/mqtt connection
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

//enable use of OTA updates
void ESPHelper::OTA_enable(){
	_useOTA = true;
	OTA_begin();
}

//begin the OTA subsystem but with a check for connectivity and enabled use of OTA
void ESPHelper::OTA_begin(){
	if(_connected && _useOTA){ArduinoOTA.begin();}
	_OTArunning = true;
}

//disable use of OTA updates
void ESPHelper::OTA_disable(){
	_useOTA = false;
	_OTArunning = false;
}

//set a password for OTA updates
void ESPHelper::OTA_setPassword(char* pass){
	ArduinoOTA.setPassword(pass);
}

void ESPHelper::OTA_setHostname(char* hostname){
	ArduinoOTA.setHostname(hostname);
}


