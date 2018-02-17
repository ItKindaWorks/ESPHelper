/*    
ESPHelperFS.h
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
    


#ifndef ESPHelperFS_H
#define ESPHelperFS_H

#include "ESPHelper.h"
#include <ArduinoJson.h>
#include "FS.h"

// #define DEBUG

#ifdef DEBUG
  #define FSdebugPrint(x) Serial.print(x) //debug on
  #define FSdebugPrintln(x) Serial.println(x) //debug on
  #define DEBUG_PRINT_SPEED 250
#else
  #define FSdebugPrint(x) {;} //debug off
  #define FSdebugPrintln(x) {;} //debug off
  #define DEBUG_PRINT_SPEED 1
#endif

const uint16_t JSON_SIZE = 512;

enum validateStates {NO_CONFIG, CONFIG_TOO_BIG, CANNOT_PARSE, INCOMPLETE, GOOD_CONFIG};

class ESPHelperFS{

public:
  ESPHelperFS();
  ESPHelperFS(const char* filename);

  static bool begin();
  static void end();

  void printFile();

  

  static int8_t validateConfig(const char* filename);

  bool createConfig(const char* filename);
  bool createConfig(const netInfo* config);
  static bool createConfig(const netInfo* config, const char* filename);

  bool loadNetworkConfig();

  bool addKey(const char* keyName, const char* value);
  static bool addKey(const char* keyName, const char* value, const char* filename);

  String loadKey(const char* keyName);
  static String loadKey(const char* keyName, const char* filename);

  netInfo getNetInfo();

  static bool createConfig(  const char* filename,
                      const char* _ssid, 
                      const char* _networkPass, 
                      const char* _deviceName, 
                      const char* _mqttIP,
                      const char* _mqttUser,
                      const char* _mqttPass,
                      const int _mqttPort,
                      const char* _otaPass,
		      const char* _willTopic,
		      const char* _willMessage,
		      const int _willQoS,
		      const int _willRetain);

  void printFSinfo();

  
  static StaticJsonBuffer<JSON_SIZE> *_tmpBufPtr;
private:
  static bool loadFile(const char* filename, std::unique_ptr<char[]> &buf);



  char ssid[64];
  char netPass[32];
  char hostName[32];
  char mqtt_ip[64];
  char mqttUser[32];
  char mqttPass[32];
  char mqttPort[16];
  char otaPass[32];
  char willTopic[64];
  char willMessage[64];
  char willQoS[4];
  char willRetain[4];

  netInfo _networkData;

  const char* _filename;

  static bool saveConfig(JsonObject& json, const char* filename);

  const netInfo defaultConfig = { mqttHost : "0.0.0.0",     //can be blank if not using MQTT
                                mqttUser : "user",   //can be blank
                                mqttPass : "pass",   //can be blank
                                mqttPort : 1883,         //default port for MQTT is 1883 - only change if needed.
                                ssid : "networkSSID", 
                                pass : "networkPass",
                                otaPassword : "otaPass",
                                hostname : "NEW-ESP8266",
				willTopic : "defaultWillTopic",
				willMessage : "",
				willQoS : 1,
				willRetain : 1}; 
};

#endif


