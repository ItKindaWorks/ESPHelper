/*    
    Copyright (c) 2017 ItKindaWorks All right reserved.
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

#define TOPIC "/your/mqtt/topic"
#define STATUS TOPIC "/status"	//dont change this - this is for the status topic which is whatever your mqtt topic is plus /status (ex /home/light/status)

#define NETWORK_HOSTNAME "YOUR OTA HOST NAME"
#define OTA_PASSWORD "YOUR OTA PASSWORD"

#define RELAY_PIN 3		//rx pin on esp
#define BLINK_PIN 1		//tx/led on esp-01
#define BUTTON_PIN 0


char* relayTopic = TOPIC;
char* statusTopic = STATUS;
char* hostnameStr = NETWORK_HOSTNAME;
char* otaPassword = OTA_PASSWORD;

//initialized the pins
const int buttonPin = BUTTON_PIN;
const int relayPin = RELAY_PIN;
const int blinkPin = BLINK_PIN;

//this is the current state of the relay
bool currentState = false;

//flag to mark when a rising edge has been detected
bool risingEdge = false;

//keeps track of the previous button state for edge detection
bool lastButtonState = false;

//set this info for your own network
netInfo homeNet = {	.mqttHost = "YOUR MQTT-IP",			//can be blank if not using MQTT
					.mqttUser = "YOUR MQTT USERNAME", 	//can be blank
					.mqttPass = "YOUR MQTT PASSWORD", 	//can be blank
					.mqttPort = 1883,					//default port for MQTT is 1883 - only chance if needed.
					.ssid = "YOUR SSID", 
					.pass = "YOUR NETWORK PASS"};

ESPHelper myESP(&homeNet);


void setup() {

	//setup Arduino OTA
	myESP.OTA_enable();
	myESP.OTA_setPassword(otaPassword);
	myESP.OTA_setHostnameWithVersion(hostnameStr);

	//enable the connection heartbeat
	myESP.enableHeartbeat(blinkPin);

	//add a subscription to the relatTopic
	myESP.addSubscription(relayTopic);

	//start ESPHelper
	myESP.begin();

	//add in the MQTT callback
	myESP.setCallback(callback);

	//set the button as an input
 	pinMode(buttonPin, INPUT);

 	//set the relay pin as an output and set to off
 	pinMode(relayPin, OUTPUT);
 	digitalWrite(relayPin, LOW);
}

	
void loop(){
	if(myESP.loop() == FULL_CONNECTION){

		//read the state of the button (LOW is pressed)
		bool buttonState = digitalRead(buttonPin);

		//if the button is pressed (LOW) and previously was not pressed(HIGH)
		if(buttonState == LOW && lastButtonState == HIGH){	

			//debounce the signal and if the button is still pressed, mark as a rising edge
			delay(50);
			if(digitalRead(buttonPin) == LOW){
				lastButtonState = LOW;			
				risingEdge = true;
			}
			
		}

		//else if  the button is not pressed and set lastButtonState to HIGH and reset the rising edge flag
		else if(buttonState == HIGH){		
			lastButtonState = HIGH;
			risingEdge = false;
		}


		//rising edge detected
		if(risingEdge){

			//set the state to the opposite of the current state
			setState(!currentState);

			//reset the rising edge flag
			risingEdge = false;
			
		}
	}
	yield();
}


void callback(char* topic, byte* payload, unsigned int length) {
	if(payload[0] == '1'){
		setState(true);
	}
	else{
		setState(false);
	}
}

//sets a new state for the relay and publishes to MQTT
void setState(bool newState){

	//dont do anything unless the newState and currentState are different
	if(newState != currentState){

		//set the current state
		currentState = newState;

		//publish to the MQTT status topic
		if(currentState){myESP.publish(statusTopic, "1", true);}
		else{myESP.publish(statusTopic, "0", true);}

		//set the relay on or off
		digitalWrite(relayPin, currentState);
	}
}











