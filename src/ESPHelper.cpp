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




void printNetInfo(const NetInfo *net, const char* header, bool printMQTT, bool printWill){
	debugPrintln(header);
	debugPrint("Hostname: ");
	debugPrintln(net->getHostname());
	debugPrint("SSID: ");
	debugPrintln(net->getSsid());
	debugPrint("PASS: ");
	debugPrintln(net->getPass());
	if (printMQTT) {
		debugPrint("MQTT Host: ");
		debugPrintln(net->getMqttHost());
		debugPrint("MQTT User: ");
		debugPrintln(net->getMqttUser());
		debugPrint("MQTT Pass: ");
		debugPrintln(net->getMqttPass());
		debugPrint("MQTT Port: ");
		debugPrintln(net->getMqttPort());
	}
	if (printWill) {
		debugPrint("Will Topic: ");
		debugPrintln(net->getMqttWillTopic());
		debugPrint("Will Message: ");
		debugPrintln(net->getMqttWillMessage());
		debugPrint("Will QoS: ");
		debugPrintln(net->getMqttWillQoS());
		debugPrint("Will Retain: ");
		debugPrintln(net->getMqttWillRetain() ? "true" : "false");
	}
}

/*
empty initializer wrapper
used for creating instances of ESPHelper that will be filled in later from a loaded config file

input: NA
output: NA
*/
ESPHelper::ESPHelper(){
	_currentNet.setSsid("");
	_currentNet.setPass("");
	_currentNet.setMqttHost("");
	_currentNet.setMqttUser("");
	_currentNet.setMqttPass("");
	_currentNet.setMqttPort(1883);
	_currentNet.setMqttWillTopic("defaultWillTopic");
	_currentNet.setMqttWillMessage("offline");
	_currentNet.setMqttWillQoS (0);
	_currentNet.setMqttWillRetain(true);
	init();
}


/*
initializer wrapper with single NetInfo network

input: NetInfo ptr
output: NA
*/
ESPHelper::ESPHelper(const NetInfo *startingNet, bool storeLocal){
	if (startingNet) {
		startingNet->cloneTo(_currentNet, storeLocal);
	} 
	//if the starting net is null then we set the default values and store locally
	else {
		_currentNet.setSsid("");
		_currentNet.setPass("");
		_currentNet.setMqttHost("");
		_currentNet.setMqttUser("");
		_currentNet.setMqttPass("");
		_currentNet.setMqttPort(1883);
		_currentNet.setMqttWillTopic("defaultWillTopic");
		_currentNet.setMqttWillMessage("offline");
		_currentNet.setMqttWillQoS (0);
		_currentNet.setMqttWillRetain(true);
	}
	
	init();
	
}




/*
initialize the NetInfo data and reset wifi. set hopping and OTA to off

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
void ESPHelper::init(){
	//diconnect from and previous wifi networks
    WiFi.softAPdisconnect();
	WiFi.disconnect();
	
	//validate various bits of network/MQTT info
	validateConfig();
}



/*
checks which parts of a NetInfo have been filled out
and updates internal flags noting which are set

input: NA
output: NA
*/
void ESPHelper::validateConfig(){
	//network pass
	if(_currentNet.getPass()[0] == '\0'){_passSet = false;}
	else{_passSet = true;}

	//ssid
	if(_currentNet.getSsid()[0] == '\0'){_ssidSet = false;}
	else{_ssidSet = true;}

	//mqtt host
	if(_currentNet.getMqttHost()[0] == '\0'){_mqttSet = false;}
	else{_mqttSet = true;}

	//mqtt port
  	if(_currentNet.getMqttPort() == 0){_currentNet.setMqttPort(1883);}

  	//mqtt username
	if(_currentNet.getMqttUser()[0] == '\0'){_mqttUserSet = false;}
	else{_mqttUserSet = true;}

	//mqtt password
	if(_currentNet.getMqttPass()[0] == '\0'){_mqttPassSet = false;}
	else{_mqttPassSet = true;}

	//Will Topic
	if(_currentNet.getMqttWillTopic()[0] == '\0'){_willTopicSet = false;}
	else{_willTopicSet = true;}

	//Will Message
	if(_currentNet.getMqttWillMessage()[0] == '\0'){_willMessageSet = false;}
	else{_willMessageSet = true;}

}



bool ESPHelper::begin(const NetInfo *startingNet, bool storeLocal){
	// If a starting net is provided
	if (startingNet) {
		startingNet->cloneTo(_currentNet, storeLocal);
	} 

	//if the starting net is null then we set the default values and store locally
	else {
		_currentNet.setSsid("");
		_currentNet.setPass("");
		_currentNet.setMqttHost("");
		_currentNet.setMqttUser("");
		_currentNet.setMqttPass("");
		_currentNet.setMqttPort(1883);
		_currentNet.setMqttWillTopic("defaultWillTopic");
		_currentNet.setMqttWillMessage("offline");
		_currentNet.setMqttWillQoS (0);
		_currentNet.setMqttWillRetain(true);
	}

	init();
	return begin();
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

		// printNetInfo(&_currentNet, "Pre wifi begin", _mqttSet, _willTopicSet);
		if(_passSet){WiFi.begin(_currentNet.getSsid(), _currentNet.getPass());}
		else{WiFi.begin(_currentNet.getSsid());}

		WiFi.setAutoReconnect(true);
		WiFi.setSleep(false);
		

		//as long as an mqtt ip has been set create an instance of PubSub for client
		if(_mqttSet){
			client.setServer(_currentNet.getMqttHost(), _currentNet.getMqttPort());

			//set the mqtt message callback if needed
			if(_mqttCallbackSet){client.setCallback(_mqttCallback);}
		}

		//define a dummy instance of mqtt so that it is instantiated if no mqtt ip is set
		else{
			client.setServer("192.0.2.0", _currentNet.getMqttPort());
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
			// printNetInfo(&_currentNet, "ESPHelper Begin reconnect loop", _mqttSet, _willTopicSet);
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
	createSafeString(topicStr, MAX_TOPIC_LENGTH);

	//loop through all subscriptions
	for(int i = 0; i < MAX_SUBSCRIPTIONS; i++){
		//if an element is used, check for it being the one we want to remove
		if(_subscriptions[i].isUsed){
			createSafeString(subStr, MAX_TOPIC_LENGTH);
			subStr = _subscriptions[i].topic;
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



bool ESPHelper::publishJson(const char* topic, JsonDocument& doc, bool retain){

	const size_t MAX_CHUNK_SIZE = 128; // Define a suitable chunk size
	
	//figure out the correct size
	size_t dataSize = measureJsonPretty(doc);

	if(dataSize < 1023){
		//create & fill
		uint8_t* buf = new uint8_t[dataSize+1];
		if (!buf) {
			return false; // Handle memory allocation failure
		}
		size_t payloadLength = serializeJsonPretty(doc, buf, dataSize+1);
		if (payloadLength == 0) {
			delete[] buf;
			return false; // Handle serialization failure
		}
		size_t bytesSent = 0;

		// Start publishing
		if (!client.beginPublish(topic, dataSize, retain)) {
			delete[] buf;
			return false;
		}

		while (bytesSent < payloadLength) {
			size_t chunkSize = min(payloadLength - bytesSent, MAX_CHUNK_SIZE);
			size_t result = client.write((const uint8_t*)(buf + bytesSent), chunkSize);
			if (result != chunkSize) {
				delete[] buf;
				client.endPublish(); // Ensure to end publish on error
				return false; // Handle error
			}
			bytesSent += result;
		}

		//cleanup
		delete[] buf;
		buf = NULL;

		return client.endPublish();
	}
	
	return false;
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
					client.setServer(_currentNet.getMqttHost(), _currentNet.getMqttPort());
					
					
					if(_useSecureClient){client.setClient(wifiClientSecure);}
					else{client.setClient(wifiClient);}


					int connected = 0;

					//connect to mqtt with user/pass
					if (_mqttUserSet && _willMessageSet && _willTopicSet) {
						debugPrintln(" - Using user & last will");
						debugPrintln(String("\t Client Name: " + String(_clientName.c_str())));
						debugPrintln(String("\t User Name: " + String(_currentNet.getMqttUser())));
						debugPrintln(String("\t Password: " + String(_currentNet.getMqttPass())));
						debugPrintln(String("\t Will Topic: " + String(_currentNet.getMqttWillTopic())));
						debugPrintln(String("\t Will QOS: " + String(_currentNet.getMqttWillQoS())));
						debugPrintln(String("\t Will Retain?: " + String(_currentNet.getMqttWillRetain())));
						debugPrintln(String("\t Will Message: " + String(_currentNet.getMqttWillMessage())));
						connected = client.connect(
							(char*) _clientName.c_str(),
							 _currentNet.getMqttUser(), 
							 _currentNet.getMqttPass(), 
							 _currentNet.getMqttWillTopic(), 
							 _currentNet.getMqttWillQoS(), 
							 _currentNet.getMqttWillRetain(), 
							 _currentNet.getMqttWillMessage());
					}

					//connect to mqtt without credentials
					else if (!_mqttUserSet && _willMessageSet && _willTopicSet) {
						debugPrintln(" - Using last will");
						debugPrintln(String("\t Client Name: " + String(_clientName.c_str())));
						debugPrintln(String("\t Will Topic: " + String(_currentNet.getMqttWillTopic())));
						debugPrintln(String("\t Will QOS: " + String(_currentNet.getMqttWillQoS())));
						debugPrintln(String("\t Will Retain?: " + String(_currentNet.getMqttWillRetain())));
						debugPrintln(String("\t Will Message: " + String(_currentNet.getMqttWillMessage())));
						connected = client.connect(
							(char*) _clientName.c_str(), 
							_currentNet.getMqttWillTopic(), 
							_currentNet.getMqttWillQoS(), 
							_currentNet.getMqttWillRetain(), 
							_currentNet.getMqttWillMessage()
						);
					} else if (_mqttUserSet && !_willMessageSet) {
						debugPrintln(" - Using user");
						debugPrintln(String("\t Client Name: " + String(_clientName.c_str())));
						debugPrintln(String("\t User Name: " + String(_currentNet.getMqttUser())));
						debugPrintln(String("\t Password: " + String(_currentNet.getMqttPass())));
						connected = client.connect(
							(char*) _clientName.c_str(), 
							_currentNet.getMqttUser(), 
							_currentNet.getMqttPass()
						);
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
				else if (timeout >= 5) {
					debugPrintln(" -- Failed to connect to MQTT after 5 attempts. Giving up.");
					_connectionStatus = WIFI_ONLY;
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
	if(_passSet && _ssidSet){WiFi.begin(_currentNet.getSsid(), _currentNet.getPass());}
	else if(_ssidSet){WiFi.begin(_currentNet.getSsid());}
	else{WiFi.begin("NO_SSID_SET");}
	
	WiFi.setSleep(false);
	//#ifdef ESP32
	WiFi.setAutoReconnect(true);
	//#endif

	debugPrintln("\tSetting new MQTT server");
	//setup the mqtt broker info
	if(_mqttSet){client.setServer(_currentNet.getMqttHost(), _currentNet.getMqttPort());}
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

  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
		   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}



/*
return the current NetInfo state

input: NA
output:
	NetInfo of the current network
*/
NetInfo* ESPHelper::getNetInfo(){
	return &_currentNet;
}


/*
return the current SSID

input: NA
output:
	char ptr referencing the current SSID	
*/
const char* ESPHelper::getSSID(){
	if(_ssidSet && _connectionStatus != BROADCAST){return _currentNet.getSsid();}
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
	_currentNet.setSsid(ssid);
	_ssidSet = true;
}


/*
return the current network password

input: NA
output:
	char ptr to the current network pass
*/
const char* ESPHelper::getPASS(){
	if(_passSet && _connectionStatus != BROADCAST){return _currentNet.getPass();}
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
	_currentNet.setPass(pass);
	_passSet = true;
}


/*
return the current MQTT server IP

input: NA
output:
	char ptr to the current MQTT IP or Hostname
*/
const char* ESPHelper::getMQTTIP(){
	if(_mqttSet){return _currentNet.getMqttHost();}
	return "MQTT IP NOT SET";
}


/*
set a new MQTT server IP or Hostname - does not automatically disconnect from current network/server if already connected

input:
	char ptr to a new MQTT IP or Hostname str
output: NA
*/
void ESPHelper::setMQTTIP(const char* mqttIP){
	_currentNet.setMqttHost(mqttIP);
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
	_currentNet.setMqttHost(mqttIP);
	_currentNet.setMqttUser(mqttUser);
	_currentNet.setMqttPass(mqttPass);
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
	_currentNet.setMqttWillTopic(willTopic);
	_currentNet.setMqttWillMessage(willMessage);
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
void ESPHelper::setWill(const char *willTopic, const char *willMessage, const int willQoS, const bool willRetain){
	_currentNet.setMqttWillTopic(willTopic);
	_currentNet.setMqttWillMessage(willMessage);
	_currentNet.setMqttWillQoS(willQoS);
	_currentNet.setMqttWillRetain(willRetain);
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
