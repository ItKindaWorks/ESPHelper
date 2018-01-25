/*    
MQTT_Will.ino
Copyright (c) 2018 Sk4zz All right reserved.
github.com/Sk4zz

This file is part of ESPHelper
github.com/ItKindaWorks

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


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "ESPHelper.h"

#define INTERNAL_LED 2        //Set Pin Number for the internal LED this is for ESP-12E
#define STATUS_TOPIC "test/status"        //Set the default topic to publish status messages


// Update these with values suitable for your network.
netInfo homeNet = {  .mqttHost = "192.168.2.3",     //can be blank if not using MQTT
          .mqttUser = "",   //can be blank
          .mqttPass = "",   //can be blank
          .mqttPort = 1883,         //default port for MQTT is 1883 - only chance if needed.
          .ssid = "",
          .pass = "",
          .otaPassword = "",  //password for OTA flashing. can be left blank
          .hostname ="ESP8266",     //Hostname of the board
          .willTopic = STATUS_TOPIC,      //Topic for Last Will
          .willMessage = "offline",       //Last Will message
          .willQoS = 1,                   //QoS of last will message
          .willRetain = 1};               //Retain flag for last will message

ESPHelper myESP(&homeNet);

bool MQTT_Connection;

bool checkMqttConnection() {
  // Check if the Connection status has changed.
  // If Status changed to FULL_CONNECTION publish "online" status message on STATUS_TOPIC
  // Toggle the INTERNAL_LED
  if( MQTT_Connection != (myESP.getStatus() == FULL_CONNECTION) ){
    MQTT_Connection = (myESP.getStatus() == FULL_CONNECTION);
    if (MQTT_Connection) {
      Serial.print("Full Connection established. publishing status on Topic ");
      Serial.println(STATUS_TOPIC);
      myESP.publish(STATUS_TOPIC, "online", true);      
    }
    digitalWrite(INTERNAL_LED, !MQTT_Connection);
  }  
}


void setup() {
  Serial.begin(115200);

  pinMode(INTERNAL_LED, OUTPUT);     // Initialize the INTERNAL_LED pin as an output
  digitalWrite(INTERNAL_LED, HIGH);   // Switch INTERNAL_LED OFF
  MQTT_Connection = false;
  
  myESP.OTA_enable();
  myESP.addSubscription("test/incoming");
  myESP.setMQTTCallback(callback);
  myESP.begin();
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void loop() {
  myESP.loop();  //run the loop() method as often as possible - this keeps the network services running

  checkMqttConnection();
  
  yield();
}
