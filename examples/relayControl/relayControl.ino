/*    
    Copyright (c) 2016 ItKindaWorks All right reserved.
    github.com/ItKindaWorks

    This file is part of RelayControl

    RelayControl is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RelayControl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RelayControl.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
	This is a simple MQTT relay/light controller program for the ESP8266.
	By sending a '1' or '0' to the relayTopic the relayPin can be toggled
	on or off. This program also posts a status update to the status topic
	which is the relayTopic plus "/status" (ex. if the relayTopic
	is "/home/light" then the statusTopic would be "home/light/status")
*/

#include "ESPHelper.h"

#define TOPIC "/your/mqtt/topic"
#define STATUS TOPIC "/status"	//dont change this - this is for the status topic which is whatever your mqtt topic is plus /status (ex /home/light/status)

#define NETWORK_HOSTNAME "YOUR OTA HOST NAME"
#define OTA_PASSWORD "YOUR OTA PASSWORD"

#define RELAY_PIN 3		//rx pin on esp
#define BLINK_PIN 1		//tx/led on esp-01


char* relayTopic = TOPIC;
char* statusTopic = STATUS;
char* hostnameStr = NETWORK_HOSTNAME;

const int relayPin = RELAY_PIN;
const int blinkPin = BLINK_PIN;		//tx pin on esp

//set this info for your own network
netInfo homeNet = {.name = "NETWORK NICKNAME", .mqtt = "YOUR MQTT-IP", .ssid = "YOUR SSID", .pass = "YOUR NETWORK PASS"};

ESPHelper myESP(&homeNet);

void setup() {
	//setup ota
	myESP.OTA_enable();
	myESP.OTA_setPassword(OTA_PASSWORD);
	myESP.OTA_setHostnameWithVersion(hostnameStr);
	

	//setup the rest of ESPHelper
	myESP.enableHeartbeat(blinkPin);	//comment out to disable the heartbeat
	myESP.addSubscription(relayTopic);	//add the relay topic to the subscription list
	myESP.begin();
	myESP.setCallback(callback);
	

	pinMode(relayPin, OUTPUT);
    delay(100);
}


void loop(){
	//loop ESPHelper and wait for commands from mqtt
	myESP.loop();
	yield();
}


//mqtt callback
void callback(char* topic, byte* payload, unsigned int length) {
	String topicStr = topic;

	//if the payload from mqtt was 1, turn the relay on and update the status topic with 1
	if(payload[0] == '1'){
		digitalWrite(relayPin, HIGH);
		myESP.publish(statusTopic, "1",true);
	}

	//else turn the relay off and update the status topic with 0
	else if (payload[0] == '0'){
		digitalWrite(relayPin, LOW);
		myESP.client.publish(statusTopic, "0", true);
	}

}
