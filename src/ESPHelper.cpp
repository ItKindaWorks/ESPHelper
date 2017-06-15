/*    
    ESPHelper.cpp
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


#include "ESPHelper.h"

//empy initializer 
ESPHelper::ESPHelper(){	}

//initializer with single netInfo network
ESPHelper::ESPHelper(netInfo *startingNet){	
	WiFi.softAPdisconnect();
	WiFi.disconnect();	
	_currentNet = *startingNet;

	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}

  	if(_currentNet.mqttPort == 0){_currentNet.mqttPort = 1883;}
  
	if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}

	_hoppingAllowed = false;

	_useOTA = false;
}


//initializer with netInfo array and index
ESPHelper::ESPHelper(netInfo *netList[], uint8_t netCount, uint8_t startIndex){	
	WiFi.softAPdisconnect();
	WiFi.disconnect();	
	_netList = netList;
	_netCount = netCount;
	_currentIndex = startIndex;

	_hoppingAllowed = true;

	_useOTA = false;

	_currentNet = *netList[constrain(_currentIndex, 0, _netCount)];

	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}

  	if(_currentNet.mqttPort == 0){_currentNet.mqttPort = 1883;}
  
	if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}

}

//initializer with single network information
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP){	
    WiFi.softAPdisconnect();
	WiFi.disconnect();
	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqttHost= mqttIP;
	_currentNet.mqttPort = 1883;

	_hoppingAllowed = false;

	_useOTA = false;

	_mqttPassSet = false;
	_mqttUserSet = false;

	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}
}

//initializer with single network information (MQTT user/pass)
ESPHelper::ESPHelper(const char *ssid, const char *pass, const char *mqttIP, const char *mqttUser, const char *mqttPass, const int mqttPort){
	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqttHost= mqttIP;
	_currentNet.mqttUser = mqttUser;
	_currentNet.mqttPass = mqttPass;
	_currentNet.mqttPort = mqttPort;

	_hoppingAllowed = false;

	_useOTA = false;

	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	

	if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}
  
  
	if(_currentNet.mqttUser[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	if(_currentNet.mqttPass[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}
}

//initializer with single network information
ESPHelper::ESPHelper(const char *ssid, const char *pass){	
    WiFi.softAPdisconnect();
	WiFi.disconnect();
	_currentNet.ssid = ssid;
	_currentNet.pass = pass;
	_currentNet.mqttHost= '\0';
	_currentNet.mqttPort = 1883;

	_hoppingAllowed = false;

	_useOTA = false;

	_mqttSet = false;
	_mqttUserSet = false;
	_mqttPassSet = false;


	if(_currentNet.pass[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}	
}



//start the wifi & mqtt systems and attempt connection (currently blocking)
	//true on: parameter check validated
	//false on: parameter check failed
bool ESPHelper::begin(){	
	if(_ssidSet){
		// Generate client name based on MAC address and last 8 bits of microsecond counter
		_clientName += "esp8266-";
		uint8_t mac[6];
		WiFi.macAddress(mac);
		_clientName += macToStr(mac);

		WiFi.mode(WIFI_STA);
		if(_passSet){WiFi.begin(_currentNet.ssid, _currentNet.pass);}
		else{WiFi.begin(_currentNet.ssid);}

		//as long as an mqtt ip has been set create an instance of PubSub for client
		if(_mqttSet){client = PubSubClient(_currentNet.mqttHost, _currentNet.mqttPort, wifiClient);}

		//define a dummy instance of mqtt so that it is instantiated if no mqtt ip is set
		else{client = PubSubClient("192.0.2.0", _currentNet.mqttPort, wifiClient);}

		
		//ota event handlers
		ArduinoOTA.onStart([]() {/* ota start code */});
		ArduinoOTA.onEnd([]() {
			WiFi.softAPdisconnect();
			WiFi.disconnect();
			int timeout = 0;
			while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
				delay(10);
				timeout++;
			}
		});
		ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {/* ota progress code */});
		ArduinoOTA.onError([](ota_error_t error) {/* ota error code */});

		int timeout = 0;	//counter for begin connection attempts
		while (((!client.connected() && _mqttSet) || WiFi.status() != WL_CONNECTED) && timeout < 200 ) {	//max 2 sec before timeout
			reconnect();
			delay(10);
			timeout++;
		}

		OTA_begin();
		
		_hasBegun = true;
		return true;
	}
	return false;
}

void ESPHelper::end(){
	OTA_disable();
	WiFi.softAPdisconnect();
	WiFi.disconnect();

	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}
}

void ESPHelper::broadcastMode(const char* ssid, const char* password, const IPAddress ip){
	WiFi.softAPdisconnect();
	WiFi.disconnect();
	int timeout = 0;
	while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
		delay(10);
		timeout++;
	}
	WiFi.mode(WIFI_AP);
	WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
	WiFi.softAP(ssid, password);
	//WiFi.softAPIP(*ip);
	//WiFi.begin(_currentNet.ssid, _currentNet.pass);

	_connectionStatus = BROADCAST;
}

void ESPHelper::disableBroadcast(){
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

//main loop - should be called as often as possible - handles wifi/mqtt connection and mqtt handler
	//true on: network/server connected
	//false on: network or server disconnected
int ESPHelper::loop(){	
	if(_ssidSet){

		if (((_mqttSet && !client.connected()) || setConnectionStatus() < WIFI_ONLY) && _connectionStatus != BROADCAST) {
			reconnect();
		}

		//run the wifi loop as long as the connection status is at a minimum of BROADCAST
		if(_connectionStatus >= BROADCAST){
			
			if(_connectionStatus == FULL_CONNECTION){client.loop();}
			
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
	}

	return -1;
}

//subscribe to a speicifc topic (does not add to topic list)
	//true on: subscription success
	//false on: subscription failed (either from PubSub lib or network is disconnected)
bool ESPHelper::subscribe(const char* topic, int qos){		
	if(_connectionStatus == FULL_CONNECTION){
		bool returnVal = client.subscribe(topic, qos);
		client.loop();
		return returnVal;
	}
	else{return false;}
}

//add a topic to the list of subscriptions and attempt to subscribe to the topic on the spot
	//true on: subscription added to list (does not guarantee that the topic was subscribed to, only that it was added to the list)
	//false on: subscription not added to list
bool ESPHelper::addSubscription(const char* topic){	
	bool subscribed = false;
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed == false){
			_subscriptions[i].topic = topic;
			_subscriptions[i].isUsed = true;
			subscribed = true;
			break;
		}
	}
	if(subscribed){subscribe(topic, _qos);}
	

	return subscribed;
}

//loops through list of subscriptions and attempts to subscribe to all topics
void ESPHelper::resubscribe(){	
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		if(_subscriptions[i].isUsed){
			subscribe(_subscriptions[i].topic, _qos);
			yield();
		}
	}
}

//attempts to remove a topic from the topic list
	//true on: subscription removed from list (does not guarantee that the topic was unsubscribed from, only that it was removed from the list)
	//false on: topic was not found in list and therefore cannot be removed
bool ESPHelper::removeSubscription(const char* topic){	
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
void ESPHelper::publish(const char* topic, const char* payload){		
	publish(topic, payload, false);
}

//publish to a specified topic with a given retain level
void ESPHelper::publish(const char* topic, const char* payload, bool retain){		
	client.publish(topic, payload, retain);
}

//set the callback function for MQTT
	//true on: mqtt has been initialized
	//false on: mqtt not been inistialized
bool ESPHelper::setCallback(MQTT_CALLBACK_SIGNATURE){	
	if(_hasBegun && _mqttSet) {
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

	if(reconnectMetro.check() && _connectionStatus != BROADCAST){
		debugPrintln("Attempting Conn...");
		//attempt to connect to the wifi if connection is lost
		if(WiFi.status() != WL_CONNECTED){
			_connectionStatus = NO_CONNECTION;
			// _connected = false;
			debugPrint(".");
			tryCount++;
			if(tryCount == 20){
				changeNetwork();
				tryCount = 0;
				return;
			}
		}

		// make sure we are connected to WIFI before attemping to reconnect to MQTT
		//----note---- maybe want to reset tryCount whenever we succeed at getting wifi connection?
		if(WiFi.status() == WL_CONNECTED){
			debugPrintln("\n---WIFI Connected!---");
			_connectionStatus = WIFI_ONLY;

			if(_mqttSet){

				int timeout = 0;	//allow a max of 10 mqtt connection attempts before timing out
				while (!client.connected() && timeout < 10) {
					debugPrint("Attemping MQTT connection");

					
					int connected = 0;
					if (_mqttUserSet) {
						connected = client.connect((char*) _clientName.c_str(), _currentNet.mqttUser, _currentNet.mqttPass);
					}
					else{
						connected = client.connect((char*) _clientName.c_str());
					}

					//if connected, subscribe to the topic(s) we want to be notified about
					if (connected) {
						debugPrintln(" -- Connected");
						// _connected = true;
						_connectionStatus = FULL_CONNECTION;
						resubscribe();
					}
					else{
						debugPrintln(" -- Failed");
						// _connected = false;
					}
					timeout++;

				}

				if(timeout >= 10 && !client.connected()){	//if we still cant connect to mqtt after 10 attempts increment the try count
					tryCount++;
					if(tryCount == 20){
						changeNetwork();
						tryCount = 0;
						return;
					}
				}
			}


		}

		reconnectMetro.reset();
	}
}

int ESPHelper::setConnectionStatus(){
	int returnVal = NO_CONNECTION;

	if(_connectionStatus != BROADCAST){

		if(WiFi.status() == WL_CONNECTED){
			returnVal = WIFI_ONLY;
			if(client.connected()){
				returnVal = FULL_CONNECTION;
			}
		}
	}
	else{
		returnVal = BROADCAST;
	}
	_connectionStatus = returnVal;
	return returnVal;
}

//changes the current network settings to the next listed network if network hopping is allowed
void ESPHelper::changeNetwork(){	

	if(_hoppingAllowed){
		_currentIndex++;
		if(_currentIndex >= _netCount){_currentIndex = 0;}

		_currentNet = *_netList[_currentIndex];

		if(_currentNet.pass[0] == '\0'){_passSet = false;}
		else{_passSet = true;}

		if(_currentNet.ssid[0] == '\0'){_ssidSet = false;}
		else{_ssidSet = true;}	

		if(_currentNet.mqttHost[0] == '\0'){_mqttSet = false;}
		else{_mqttSet = true;}

		debugPrint("Trying next network: ");
		debugPrintln(_currentNet.ssid);

		updateNetwork();
	}

	
	// debugPrintln("\tDisconnecting from WiFi");
	// WiFi.disconnect();
	// debugPrintln("\tAttempting to begin on new network");
	// WiFi.begin(_currentNet.ssid, _currentNet.pass);
	// debugPrintln("\tSetting new MQTT server");
	// client.setServer(_currentNet.mqtt, 1883);
	// debugPrintln("\tDone - Ready for next reconnect attempt");
	//ALL THIS COMMENTED CODE IS HANDLED BY updateNetwork()
}

void ESPHelper::updateNetwork(){
	debugPrintln("\tDisconnecting from WiFi");
	WiFi.disconnect();
	debugPrintln("\tAttempting to begin on new network");

	WiFi.mode(WIFI_STA);
	if(_passSet && _ssidSet){WiFi.begin(_currentNet.ssid, _currentNet.pass);}
	else if(_ssidSet){WiFi.begin(_currentNet.ssid);}
	else{WiFi.begin("NO_SSID_SET");}

	debugPrintln("\tSetting new MQTT server");
	if(_mqttSet){client.setServer(_currentNet.mqttHost, 1883);}
	else{client.setServer("192.0.2.0", 1883);}
	
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
	_mqttUserSet = true;
}

//change the current network info to a new *netInfo - does not automatically disconnect from current network if already connected
void ESPHelper::setNetInfo(netInfo *newNetwork){ 	
	_currentNet = *newNetwork;
	_ssidSet = true;
	_passSet = true;
	_mqttSet = true;
	_mqttUserSet = true;
}

//return the current netInfo state
netInfo* ESPHelper::getNetInfo(){	
	return &_currentNet;
}

//return the current SSID
const char* ESPHelper::getSSID(){			
	if(_ssidSet){return _currentNet.ssid;}
	return "SSID NOT SET";
}
//set a new SSID - does not automatically disconnect from current network if already connected
void ESPHelper::setSSID(const char* ssid){		
	_currentNet.ssid = ssid;
	_ssidSet = true;
}

//return the current network password
const char* ESPHelper::getPASS(){			
	if(_passSet){return _currentNet.pass;}
	return "PASS NOT SET";
}
//set a new network password - does not automatically disconnect from current network if already connected
void ESPHelper::setPASS(const char* pass){ 	
	_currentNet.pass = pass;
	_passSet = true;
}

//return the current MQTT server IP
const char* ESPHelper::getMQTTIP(){		
	if(_mqttSet){return _currentNet.mqttHost;}
	return "MQTT IP NOT SET";
}
//set a new MQTT server IP - does not automatically disconnect from current network/server if already connected
void ESPHelper::setMQTTIP(const char* mqttIP){ 
	_currentNet.mqttHost= mqttIP;
	_mqttSet = true;
}

//set a new MQTT server IP - does not automatically disconnect from current network/server if already connected
void ESPHelper::setMQTTIP(const char* mqttIP, const char* mqttUser, const char* mqttPass){
	_currentNet.mqttHost= mqttIP;
	_currentNet.mqttUser = mqttUser;
	_currentNet.mqttPass = mqttPass;
	_mqttSet = true;
	_mqttUserSet = true;
}


int ESPHelper::getMQTTQOS(){
	return _qos;

}
void ESPHelper::setMQTTQOS(int qos){
	_qos = qos;
}


String ESPHelper::getIP(){
	return WiFi.localIP().toString();
}


int ESPHelper::getStatus(){
	return _connectionStatus;
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
	if(_connectionStatus >= BROADCAST && _useOTA){
		ArduinoOTA.begin();
		_OTArunning = true;
	}
}

//disable use of OTA updates
void ESPHelper::OTA_disable(){
	_useOTA = false;
	_OTArunning = false;
}

//set a password for OTA updates
void ESPHelper::OTA_setPassword(const char* pass){
	ArduinoOTA.setPassword(pass);
}

void ESPHelper::OTA_setHostname(const char* hostname){
	strcpy(_hostname, hostname);
	ArduinoOTA.setHostname(_hostname);
}

void ESPHelper::OTA_setHostnameWithVersion(const char* hostname){
	strcpy(_hostname, hostname);
	strcat(_hostname, "----");
	strcat(_hostname, VERSION);

	ArduinoOTA.setHostname(_hostname);
}


