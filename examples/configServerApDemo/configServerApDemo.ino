/*    
configServerApDemo.ino
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
#include "ESPHelperFS.h"
#include "ESPHelperWebConfig.h"
#include "Metro.h"

netInfo config;
ESPHelper myESP;
ESPHelperWebConfig configPage(80, "/");

netInfo homeNet = { .mqttHost = "YOUR MQTT-IP",     //can be blank if not using MQTT
          .mqttUser = "YOUR MQTT USERNAME",   //can be blank
          .mqttPass = "YOUR MQTT PASSWORD",   //can be blank
          .mqttPort = 1883,         //default port for MQTT is 1883 - only chance if needed.
          .ssid = "YOUR SSID", 
          .pass = "YOUR NETWORK PASS"};

Metro connectTimeout = Metro(20000);
bool timeout = false;

const char* broadcastSSID = "ESP-Hotspot";
const char* broadcastPASS = "";
IPAddress broadcastIP = {192, 168, 1, 1};



void setup(void){
  Serial.begin(115200);

  //print some debug
  Serial.println("Starting Up - Please Wait...");
  delay(100);

  //startup the wifi and web server
  startWifi();
  configPage.begin(config.hostname);
}



void loop(void){
  //get the current status of ESPHelper
  int espHelperStatus = myESP.loop();
  manageESPHelper(espHelperStatus);

  if(espHelperStatus >= FULL_CONNECTION){
    //regular loop code goes here
  }


  delay(5);
}



void manageESPHelper(int wifiStatus){
  //if the unit is broadcasting or connected to wifi then reset the timeout vars
  if(wifiStatus == BROADCAST || wifiStatus >= WIFI_ONLY){
    connectTimeout.reset();
    timeout = false;
  }
  //otherwise check for a timeout condition and handle setting up broadcast
  else if(wifiStatus < WIFI_ONLY){
    checkForWifiTimeout();
  }
  //handle saving a new network config
  if(configPage.handle()){
    Serial.println("Saving new network config and restarting...");
    myESP.saveConfigFile(configPage.getConfig(), "/netConfig.json");
    delay(500);
    ESP.restart();
  }
}

void startWifi(){
  //check for a good config file and start ESPHelper with the file stored on the ESP
  if(ESPHelperFS::begin()){
    Serial.println("Filesystem loaded - Loading Config");
    if(ESPHelperFS::validateConfig("/netConfig.json") == GOOD_CONFIG){
      Serial.println("Config loaded");
      delay(10);
      myESP.begin("/netConfig.json");
    }
    else{
      Serial.println("Could not load config - saving new config from default values and restarting");
      delay(10);
      ESPHelperFS::createConfig(&homeNet, "/netConfig.json");
      ESPHelperFS::end();
      ESP.restart();
    }
  }
  
  //if the filesystem cannot be started, just fail over to the built in network config
  else{
    Serial.println("Could not load filesystem, proceeding with default config values");
    delay(10);
    myESP.begin(&homeNet);
  }
  config = myESP.getNetInfo();

  //setup other ESPHelper info and enable OTA updates
  myESP.setHopping(false);
  myESP.OTA_setPassword(config.otaPassword);
  myESP.OTA_setHostnameWithVersion(config.hostname);
  myESP.OTA_enable();

  Serial.println("Connecting to network");
  delay(10);
  //connect to wifi before proceeding. If cannot connect then switch to ap mode and create a network to config from
  while(myESP.loop() < WIFI_ONLY){
    checkForWifiTimeout();
    if(timeout){return;}
    delay(10);
  }

  Serial.println("Sucess!");
}

//this is more or less the same procedure as above however this is for checking during runtime
void checkForWifiTimeout(){
  if(connectTimeout.check() && !timeout){
      Serial.println("Network Connection timeout - starting broadcast (AP) mode...");
      timeout = true;
      myESP.broadcastMode(broadcastSSID, broadcastPASS, broadcastIP);
      myESP.OTA_setPassword(config.otaPassword);
      myESP.OTA_setHostnameWithVersion(config.hostname);
      myESP.OTA_enable();
    }
}


