/*    
configServerDemo.ino
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

netInfo config;
ESPHelper myESP;

ESPHelperWebConfig configPage(80, "/");

netInfo homeNet = { .mqttHost = "YOUR MQTT-IP",     //can be blank if not using MQTT
          .mqttUser = "YOUR MQTT USERNAME",   //can be blank
          .mqttPass = "YOUR MQTT PASSWORD",   //can be blank
          .mqttPort = 1883,         //default port for MQTT is 1883 - only chance if needed.
          .ssid = "YOUR SSID", 
          .pass = "YOUR NETWORK PASS",
          .otaPassword = "YOUR OTA PASS",
          .hostname = "NEW-ESP8266"}; 

void setup(void){
  //check for a good config file and start ESPHelper with the file stored on the ESP
  if(ESPHelperFS::begin()){
    if(ESPHelperFS::validateConfig("/netConfig.json") == GOOD_CONFIG){
      myESP.begin("/netConfig.json");
    }
    else{
      ESPHelperFS::createConfig(&homeNet, "/netConfig.json");
      ESPHelperFS::end();
      ESP.restart();
    }
  }
  //if the filesystem cannot be started, just fail over to the built in network config
  else{
    myESP.begin(&homeNet);
  }
  
  config = myESP.getNetInfo();

  //setup other ESPHelper info and enable OTA updates
  myESP.setHopping(false);
  myESP.OTA_setPassword(config.otaPassword);
  myESP.OTA_setHostnameWithVersion(config.hostname);
  myESP.OTA_enable();


  configPage.begin(config.hostname);
  
}

void loop(void){
  if(myESP.loop() >= FULL_CONNECTION){
    //regular loop code goes here
  }

  //handle saving a new network config
  if(configPage.handle()){
    myESP.saveConfigFile(configPage.getConfig(), "/netConfig.json");
    delay(500);
    ESP.restart();
  }

  delay(5);
}

