/*    
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

/*
	This is a simple demo of an MQTT enabled button. A button is attahed
	to pin 0 with a pull-up resistor and each time the button is pressed,
	it toggles the state and publishes the new state to the MQTT broker.
	It is also subscribed to the same topic that it publishes to so that
	the state of the toggle can be updated from the MQTT side.
*/
#include "ESPHelper.h"

#define TOPIC "/your/mqtt/topic"
#define NETWORK_HOSTNAME "YOUR OTA HOSTNAME"
#define OTA_PASSWORD "YOUR OTA PASSWORD"

#define BUTTON_PIN 0	//button on pin 0 with pull up resistor (pulled low on press)
#define BLINK_PIN 1


char* buttonTopic = TOPIC;
char* hostnameStr = NETWORK_HOSTNAME;

const int buttonPin = BUTTON_PIN;
const int blinkPin = BLINK_PIN;

bool currentState = false;

bool lastButtonState = false;

//set this info for your own network
netInfo homeNet = {.name = "NETWORK NICKNAME", .mqtt = "YOUR MQTT-IP", .ssid = "YOUR SSID", .pass = "YOUR NETWORK PASS"};
ESPHelper myESP(&homeNet);

void setup() {
	//setup ota on esphelper
	myESP.OTA_enable();
	myESP.OTA_setPassword(OTA_PASSWORD);
	myESP.OTA_setHostnameWithVersion(hostnameStr);
	
	//enable the connection heartbeat
	myESP.enableHeartbeat(blinkPin);

	//subscribe to the button topic (this allows outside control of the state of the switch)
	myESP.addSubscription(buttonTopic);

	//setup the mqtt callback function
	myESP.setCallback(callback);

	//start ESPHelper
	myESP.begin();

	//set the button pin as an input
 	pinMode(buttonPin, INPUT);
}

	
void loop(){
	if(myESP.loop() == FULL_CONNECTION){

		//read the button (low on press, high on release)
		bool buttonState = digitalRead(buttonPin);

		//if the button is pressed (LOW) and previously was not pressed(HIGH)
		if(buttonState == LOW && lastButtonState == HIGH){
			//invert the current state
			currentState = !currentState;

			//publish to mqtt based on current state
			if(currentState){
				myESP.publish(buttonTopic, "1", true);
			}
			else{
				myESP.publish(buttonTopic, "0", true);
			}

			//set the lastButtonState to LOW to prevent multiple triggers
			lastButtonState = LOW;

			//wait half a second (poor mans debounce)
			delay(500);
		}

		//else the button is not pressed so set lastButtonState to HIGH
		else{lastButtonState = HIGH;}
	}
	yield();
}


void callback(char* topic, byte* payload, unsigned int length) {

	//if the payload is '1' then set the state to true
	if(payload[0] == '1'){
		currentState = true;
	}

	//otherwise set the current state to false
	else{
		currentState = false;
	}
}











