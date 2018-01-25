/*    
ESPHelperFS.cpp
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

#include "ESPHelperFS.h"

// StaticJsonBuffer<JSON_SIZE>* ESPHelperFS::_tmpBufPtr = NULL;


ESPHelperFS::ESPHelperFS() : _filename("/netConfig.json"){
  _networkData = defaultConfig;
}

ESPHelperFS::ESPHelperFS(const char* filename){
  _filename = filename;
  _networkData = defaultConfig;
}

bool ESPHelperFS::begin(){
  //load the config file
  if (SPIFFS.begin()) {
    // printFSinfo();
    FSdebugPrintln("Loaded Filesystem");
    return true;
  }
  return false;
}

void ESPHelperFS::end(){
  FSdebugPrintln("Filesystem Unloaded");
  SPIFFS.end();
}




void ESPHelperFS::printFile(){
  // this opens the file "f.txt" in read-mode
  File f = SPIFFS.open(_filename, "r");
  
  if(f) {
    // we could open the file
    while(f.available()) {
      //Lets read line by line from the file
      String line = f.readStringUntil('\n');
      FSdebugPrintln(line);
    }

  }
  f.close();
}



void ESPHelperFS::printFSinfo(){
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  FSdebugPrint("total bytes: ");
  FSdebugPrintln(fs_info.totalBytes);
  FSdebugPrint("used bytes: ");
  FSdebugPrintln(fs_info.usedBytes);
  FSdebugPrint("block size: ");
  FSdebugPrintln(fs_info.blockSize);
  FSdebugPrint("page size: ");
  FSdebugPrintln(fs_info.pageSize);
  FSdebugPrint("max open files: ");
  FSdebugPrintln(fs_info.maxOpenFiles);
  FSdebugPrint("max path length: ");
  FSdebugPrintln(fs_info.maxPathLength);
}


//load the file from FS into var buf
bool ESPHelperFS::loadFile(const char* filename, std::unique_ptr<char[]> &buf){

  FSdebugPrint("Opening File: ");
  FSdebugPrintln(filename);
  //open file as read only
  File configFile = SPIFFS.open(filename, "r");

  //check to make sure opening was possible
  if (!configFile) {
    FSdebugPrintln("Failed to open config file");
    configFile.close();
    return false;
  }



  //make sure the config isnt too large to store in the JSON container
  size_t size = configFile.size();
  FSdebugPrint("JSON File Size: ");
  FSdebugPrintln(size);
  if (size > JSON_SIZE) { 
    FSdebugPrintln("JSON File too large - returning"); 
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> newBuf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(newBuf.get(), size);

  //move the contents of newBuf into buf
  buf = std::move(newBuf);

  //close out the file
  configFile.close();

  //sucess return true
  return true;
  
}

int8_t ESPHelperFS::validateConfig(const char* filename){
  
  //create a buffer for the file data
  std::unique_ptr<char[]> buf(new char[JSON_SIZE]);
  loadFile(filename, buf);
  StaticJsonBuffer<JSON_SIZE> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if(json.size() == 0){ return NO_CONFIG;}

  //return false if the file could not be parsed
  if (!json.success()) {
    FSdebugPrintln("JSON File corrupt");
    
    return CANNOT_PARSE;
  }

  //check to make sure all netInfo keys exist
  if(!json.containsKey("ssid") 
    || !json.containsKey("networkPass") 
    || !json.containsKey("mqttIP") 
    || !json.containsKey("mqttUSER")
    || !json.containsKey("mqttPASS")
    || !json.containsKey("mqttPORT")
    || !json.containsKey("hostname")
    || !json.containsKey("OTA_Password")){

    FSdebugPrintln("Config incomplete");
    
    return INCOMPLETE;
  }
  return GOOD_CONFIG;
}

bool ESPHelperFS::loadNetworkConfig(){

  //validate that the config file is good and if not create a new one
  if(validateConfig(_filename) != GOOD_CONFIG){
    createConfig(&defaultConfig);
    return false;
  }


  else{
    //create a buffer for the file data
    std::unique_ptr<char[]> buf(new char[JSON_SIZE]);
    loadFile(_filename, buf);
    StaticJsonBuffer<JSON_SIZE> jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());


    //copy the keys into char arrays
    strcpy(ssid, json["ssid"]);
    strcpy(netPass, json["networkPass"]);
    strcpy(hostName, json["hostname"]);
    strcpy(mqtt_ip, json["mqttIP"]);
    strcpy(otaPass, json["OTA_Password"]);
    strcpy(mqttPort, json["mqttPORT"]);
    strcpy(mqttUser, json["mqttUSER"]);
    strcpy(mqttPass, json["mqttPASS"]);

    int port = atoi(mqttPort);

    //then set that data into a netInfo object
    _networkData = {mqttHost : mqtt_ip,
                    mqttUser : mqttUser,
                    mqttPass : mqttPass,
                    mqttPort : port,
                    ssid : ssid,
                    pass : netPass,
                    otaPassword : otaPass,
                    hostname : hostName};

    
                    
    FSdebugPrintln("Reading config file with values: ");
    FSdebugPrint("MQTT Server: ");
    FSdebugPrintln(_networkData.mqttHost);
    FSdebugPrint("MQTT User: ");
    FSdebugPrintln(_networkData.mqttUser);
    FSdebugPrint("MQTT Password: ");
    FSdebugPrintln(_networkData.mqttPass);
    FSdebugPrint("MQTT Port: ");
    FSdebugPrintln(_networkData.mqttPort);
    FSdebugPrint("SSID: ");
    FSdebugPrintln(_networkData.ssid);
    FSdebugPrint("Network Pass: ");
    FSdebugPrintln(_networkData.pass);
    FSdebugPrint("Device Name: ");
    FSdebugPrintln(_networkData.hostname);
    FSdebugPrint("OTA Password: ");
    FSdebugPrintln(_networkData.otaPassword);

    // configFile.close();
  }

  
  return true;
}


//add a key to a json file
bool ESPHelperFS::addKey(const char* keyName, const char* value){
  if(_filename != ""){
    return addKey(keyName, value, _filename);
  }
  else{return false;}
}

//add a key to a json file
bool ESPHelperFS::addKey(const char* keyName, const char* value, const char* filename){
  if(!SPIFFS.exists(filename)){
    File configFile = SPIFFS.open(filename, "w");
    configFile.close();
  }

  //create a buffer for the file data
  std::unique_ptr<char[]> buf(new char[JSON_SIZE]);
  loadFile(filename, buf);
  StaticJsonBuffer<JSON_SIZE> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if(json.success()){
    //add the key to the json object
    json[keyName] = value;
    FSdebugPrint("Added Key ");
    FSdebugPrint(keyName);
    FSdebugPrint(" With Value ");
    FSdebugPrintln(value);

    //save the new config with added key
    saveConfig(json, filename);
    return true;
  }


  StaticJsonBuffer<JSON_SIZE> blankBuffer;
  JsonObject& blankJson = blankBuffer.createObject();
  if(blankJson.success()){
    //add the key to the json object
    blankJson[keyName] = value;
    FSdebugPrint("Added Key ");
    FSdebugPrint(keyName);
    FSdebugPrint(" With Value ");
    FSdebugPrintln(value);

    //save the new config with added key
    saveConfig(blankJson, filename);
    return true;
  }
    
  return false;
}

//read a key from a json file
String ESPHelperFS::loadKey(const char* keyName){
  if(_filename != ""){
    return loadKey(keyName, _filename);
  }
  else {return String();}
}

//read a key from a json file
String ESPHelperFS::loadKey(const char* keyName, const char* filename){
  static String returnString = "";

  //create a buffer for the file data
  std::unique_ptr<char[]> buf(new char[JSON_SIZE]);
  loadFile(filename, buf);
  StaticJsonBuffer<JSON_SIZE> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if(json.success()){

    //if the key does not exist then return an empty string
    if(!json.containsKey(keyName)){
      FSdebugPrintln("Key not found");
      
      return returnString;
    }

    //set the return value to that of the key
    returnString = (const char*)json[keyName];
  }

  
  //return the key (blank if file could not be opened)
  return returnString;
}







netInfo ESPHelperFS::getNetInfo(){
  return _networkData;
}

bool ESPHelperFS::createConfig(const char* filename){
  return createConfig(_filename,
                      defaultConfig.ssid, 
                      defaultConfig.pass, 
                      defaultConfig.hostname, 
                      defaultConfig.mqttHost,  
                      defaultConfig.mqttUser, 
                      defaultConfig.mqttPass, 
                      defaultConfig.mqttPort,
                      defaultConfig.otaPassword);
}


bool ESPHelperFS::createConfig(const netInfo* config){
  return createConfig(_filename,
                      config->ssid, 
                      config->pass, 
                      config->hostname, 
                      config->mqttHost,  
                      config->mqttUser, 
                      config->mqttPass, 
                      config->mqttPort,
                      config->otaPassword);
}

bool ESPHelperFS::createConfig(const netInfo* config, const char* filename){
  return createConfig(filename,
                      config->ssid, 
                      config->pass, 
                      config->hostname, 
                      config->mqttHost,  
                      config->mqttUser, 
                      config->mqttPass, 
                      config->mqttPort,
                      config->otaPassword);
}

bool ESPHelperFS::createConfig( const char* filename,
                                const char* _ssid, 
                                const char* _networkPass, 
                                const char* _deviceName, 
                                const char* _mqttIP,
                                const char* _mqttUser,
                                const char* _mqttPass,
                                const int _mqttPort,
                                const char* _otaPass) {

  FSdebugPrintln("Generating new config file with values: ");
  FSdebugPrint("SSID: ");
  FSdebugPrintln(_ssid);
  FSdebugPrint("Network Pass: ");
  FSdebugPrintln(_networkPass);
  FSdebugPrint("hostname: ");
  FSdebugPrintln(_deviceName);
  FSdebugPrint("MQTT Server: ");
  FSdebugPrintln(_mqttIP);
  FSdebugPrint("MQTT Username: ");
  FSdebugPrintln(_mqttUser);
  FSdebugPrint("MQTT Password: ");
  FSdebugPrintln(_mqttPass);
  FSdebugPrint("MQTT PORT: ");
  FSdebugPrintln(_mqttPort);
  FSdebugPrint("OTA Password: ");
  FSdebugPrintln(_otaPass);

  char portString[10];
  sprintf(portString, "%d", _mqttPort);

  FSdebugPrintln("creating json");

  //create a buffer for the file data
  std::unique_ptr<char[]> buf(new char[JSON_SIZE]);
  loadFile(filename, buf);
  StaticJsonBuffer<JSON_SIZE> jsonBuffer;

  //if a json file already exists then use that as the base
  JsonObject& json = validateConfig(filename) == GOOD_CONFIG ? jsonBuffer.parseObject(buf.get()) : jsonBuffer.createObject();

  json["ssid"] = _ssid;
  json["networkPass"] = _networkPass;
  json["hostname"] = _deviceName;
  json["mqttIP"] = _mqttIP;
  json["mqttPORT"] = portString;
  json["mqttUSER"] = _mqttUser;
  json["mqttPASS"] = _mqttPass;
  json["OTA_Password"] = _otaPass;

  FSdebugPrintln("done");

  return saveConfig(json, filename);
}

bool ESPHelperFS::saveConfig(JsonObject& json, const char* filename) {
  FSdebugPrintln("Saving File...");

  if(SPIFFS.exists(filename)){
    SPIFFS.remove(filename);
  }

  FSdebugPrintln("Opening File as write only");


  File configFile = SPIFFS.open(filename, "w");
  if (!configFile) {
    FSdebugPrintln("Failed to open config file for writing");
    return false;
  }

  FSdebugPrintln("File open now writing");
  
  json.printTo(configFile);

  FSdebugPrintln("Writing done now closing");

  configFile.close();

  FSdebugPrintln("done.");
  return true;
}



