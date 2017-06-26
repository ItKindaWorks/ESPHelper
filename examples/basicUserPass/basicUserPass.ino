/*    
    basicUserPass.ino
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
#include "Metro.h"

#define SSID            "YOUR SSID"
#define NETWORK_PASS    "YOUR NETWORK PASS"

#define MQTT_HOST       "MQTT_HOST"
#define MQTT_USERNAME   "YOUR_MQTT_USERNAME"
#define MQTT_PASS       "YOUR_MQTT_KEY"
#define MQTT_PORT       1883
#define MQTT_TOPIC      "/test"


//Metro timer for 5 second intervals (used to time publishing to MQTT)
Metro postMetro = Metro(5000);


netInfo homeNet = { .mqttHost = MQTT_HOST,
                    .mqttUser = MQTT_USERNAME,   
                    .mqttPass = MQTT_PASS,
                    .mqttPort = 1883,                   //default port for MQTT is 1883 - only chance if needed.
                    .ssid = SSID, 
                    .pass = NETWORK_PASS};

ESPHelper myESP(&homeNet);


int counter = 0;


void setup() {
	Serial.begin(115200);

    //setup ESPHelper
    myESP.setMQTTCallback(callback);
    myESP.addSubscription(MQTT_TOPIC);
	myESP.begin();
}


void loop(){

    //check and make sure that we have a full connection to both WIFI and MQTT
    //and only post if the timer has gone off
    if(myESP.loop() == FULL_CONNECTION && postMetro.check()){
        //print to the serial line
        Serial.print(counter);
        Serial.println(" Sent");

        //generate a string from our counter variable
        char pubString[10];
        itoa(counter, pubString, 10);

        //publish the data to MQTT
        myESP.publish(MQTT_TOPIC, pubString);

        //increment the counter
        counter++;  
    }


    yield();
}


void callback(char* topic, uint8_t* payload, unsigned int length) {

    //generate a new payload string that is null terminated
    char newPayload[40];
    memcpy(newPayload, payload, length);
    newPayload[length] = '\0';

    //print that info back out the the serial line
    Serial.print(newPayload);
    Serial.println(" Received");
}












