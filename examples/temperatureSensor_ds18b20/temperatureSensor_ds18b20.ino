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
	This is a simple program that periodically (10 seconds) reads a ds18b20 temperature
	sensor and publishes the result to an MQTT topic. Change the Topic/Hostname/OTA Password 
	and network settings to match your system.
*/

#include "ESPHelper.h"
#include <Metro.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_TOPIC "/your/mqtt/topic"
#define NETWORK_HOSTNAME "YOUR OTA HOSTNAME"
#define OTA_PASSWORD "YOUR OTA PASSWORD"

#define ONE_WIRE_BUS 2
#define BLINK_PIN 1

char* tempTopic = TEMP_TOPIC;
char* hostnameStr = NETWORK_HOSTNAME;
const int wireBus = ONE_WIRE_BUS;
const int blinkPin = BLINK_PIN;


//the current temperature
float currentTemp = 0;

//timer to set how often the sensor should publish to mqtt (in ms)
Metro publishTimer = Metro(10000);

//ds18b20 variables
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

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

	//start ESPHelper
	myESP.begin();

	//start the dallas temperature sensor library
	sensors.begin();
	delay(500);	
}

void loop(){
	

	while(1){
		if(myESP.loop() == FULL_CONNECTION){

			//only read/publish the temperature if the timer is up
			if(publishTimer.check()){

				sensors.requestTemperatures(); // Send the command to get temperatures
				currentTemp = sensors.getTempCByIndex(0);	//get the temperature

				//convert the float to a string
				char temperature[10];
				dtostrf(currentTemp,4,1,temperature);

				//publish to mqtt
				myESP.publish(tempTopic,temperature, true);	
			}
		}

		yield();
	}
	
}


void callback(char* topic, uint8_t* payload, unsigned int length) {

}







