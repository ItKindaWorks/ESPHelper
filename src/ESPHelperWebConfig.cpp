/*
ESPHelperWebConfig.cpp
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
#include "ESPHelperWebConfig.h"


ESPHelperWebConfig::ESPHelperWebConfig(int port, const char* URI){
  _server = new AsyncWebServer(port);
  _runningLocal = true;
  createSafeStringFromCharArray(pageURI, _pageURI);
  pageURI.clear();
  pageURI.print(URI);
  createSafeStringFromCharArray(resetURI, _resetURI);
  resetURI.clear();
}

ESPHelperWebConfig::ESPHelperWebConfig(AsyncWebServer *server, const char* URI){
  _server = server;
  _runningLocal = false;
  createSafeStringFromCharArray(pageURI, _pageURI);
  pageURI.clear();
  pageURI.print(URI);
  createSafeStringFromCharArray(resetURI, _resetURI);
  resetURI.clear();
}

ESPHelperWebConfig::~ESPHelperWebConfig(){
  if(_runningLocal){
    _server->end(); // Stop the server
    delete _server;
  }
  _server = nullptr;
}


bool ESPHelperWebConfig::begin(const char* _hostname){
  MDNS.begin(_hostname);
  return begin();
}

bool ESPHelperWebConfig::begin(){
  createSafeStringFromCharArray(resetURI, _resetURI);
  createSafeStringFromCharArray(pageURI, _pageURI);
  // Serial.printf("[ESPHelperWebConfig] Starting web config server on %s\n", pageURI.c_str());
  

  //setup server handlers
  //these handler function definitions use lambdas to pass the funtion... more information can be found here:
  //https://stackoverflow.com/questions/39803135/c-unresolved-overloaded-function-type

  _server->on(pageURI.c_str(), HTTP_GET, [this](AsyncWebServerRequest *request){ 
    //send back the config page using the handleGet function as the template processor
    request->send_P(200, "text/html", static_config_html, sizeof(static_config_html), [this](const String& var){
      return handleGet(var);
    });
  });    

  _server->on(pageURI.c_str(), HTTP_POST, [this](AsyncWebServerRequest *request){ 
    this->handlePost(request); 
  }); 

  if(_resetSet){
    // Serial.printf("[ESPHelperWebConfig] Starting reset on %s\n", resetURI.c_str());
    //if the reset URI has been set then add a handler for it
    _server->on(resetURI.c_str(), HTTP_POST, [this](AsyncWebServerRequest *request){ 
      this->handleReset(request); 
    });
  }

  if(_runningLocal){
    _server->begin(); // Actually start the server
  }


	return true;
}

void ESPHelperWebConfig::useConfig(NetInfo& config){
  config.cloneTo(_config, false);
}

void ESPHelperWebConfig::fillConfig(bool preFill){
  _preFill = preFill;
}

bool ESPHelperWebConfig::handle(){
	// _server->handleClient();
	return _configChanged;
}

NetInfo& ESPHelperWebConfig::getConfig(){
  _configChanged = false;
  return _config;
}





//main config page that allows user to enter in configuration info
String ESPHelperWebConfig::handleGet(const String& var){

  if(var == "HELPER_PAGE_URI")
    return _pageURI;
  else if(var == "HELPER_HOSTNAME")
    return _config.getHostname();
  else if(var == "HELPER_SSID")
    return _config.getSsid();
  else if(var == "HELPER_MQTT_HOST")
    return _config.getMqttHost();
  else if(var == "HELPER_MQTT_USER")
    return _config.getMqttUser();
  else if(var == "HELPER_MQTT_PORT")
    return String(_config.getMqttPort());
  else if(var == "HELPER_MQTT_WILL_TOPIC")
    return _config.getMqttWillTopic();
  else if(var == "HELPER_MQTT_WILL_MESSAGE")
    return _config.getMqttWillMessage();
  else if(var == "HELPER_MQTT_WILL_QOS")
    return String(_config.getMqttWillQoS());
  else if(var == "HELPER_MQTT_WILL_RETAIN_0")
    return _config.getMqttWillRetain() ? "" : "selected";
  else if(var == "HELPER_MQTT_WILL_RETAIN_1")
    return _config.getMqttWillRetain() ? "selected" : "";

  return String();
}

// If a POST request is made to URI /config
void ESPHelperWebConfig::handlePost(AsyncWebServerRequest *request) {

  //make sure that all the arguments exist and that at least an SSID and hostname have been entered
  if( ! request->hasArg("ssid") || ! request->hasArg("netPass")
      || ! request->hasArg("hostname") || ! request->hasArg("mqttHost")
      || ! request->hasArg("mqttUser") || ! request->hasArg("mqttPass")
      || ! request->hasArg("mqttPort") || ! request->hasArg("otaPassword")
      || request->arg("ssid") == NULL || request->arg("hostname") == NULL){ // If the POST request doesn't have username and password data

    request->send(400, "text/plain", "400: Invalid Request - Did you make sure to specify an SSID and Hostname?");  // The request is invalid, so send HTTP status 400
    return;
  }

  //if there is an mqtt user/pass/port entered then there better also be a host!
  if((request->arg("mqttUser") != NULL || request->arg("mqttPass") != NULL ||
   request->arg("mqttPort") != NULL) && request->arg("mqttHost") == NULL){

   request->send(400, "text/html",
   String("<center>\
   <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\
   <p><strong>400: Invalid Request - MQTT info specified without host</strong></p>\
   <hr />\
   <p><a href=" + String("config") + ">Open configuration page</a></p>").c_str());
    return;
  }

  //convert the Strings returned by request->arg to char arrays that can be entered into NetInfo

  if(request->arg("netPass").length() != 0 or _preFill == false){
    _config.setPass(request->arg("netPass").c_str());
  }
  if(request->arg("mqttPass").length() != 0 or _preFill == false){
    _config.setMqttPass(request->arg("mqttPass").c_str());
  }
  if(request->arg("otaPassword").length() != 0 or _preFill == false){
    _config.setOtaPassword(request->arg("otaPassword").c_str());
  }

  _config.setSsid(request->arg("ssid").c_str());
  _config.setHostname(request->arg("hostname").c_str());
  _config.setMqttHost(request->arg("mqttHost").c_str());
  _config.setMqttUser(request->arg("mqttUser").c_str());

  //the port is special because it doesnt get stored as a string so we take care of that
  if(request->arg("mqttPort") != NULL){_config.setMqttPort(request->arg("mqttPort").toInt());}
  else{_config.setMqttPort(1883);}

  // MQTT Will fields
  if (request->hasArg("mqttWillTopic")) {
    _config.setMqttWillTopic(request->arg("mqttWillTopic").c_str());
  }
  if (request->hasArg("mqttWillMessage")) {
    _config.setMqttWillMessage(request->arg("mqttWillMessage").c_str());
  }
  if (request->hasArg("mqttWillQos")) {
    _config.setMqttWillQoS(request->arg("mqttWillQos").toInt());
  }
  if (request->hasArg("mqttWillRetain")) {
    _config.setMqttWillRetain(request->arg("mqttWillRetain").toInt() == 1);
  }

  //tell the user that the config is loaded in and the module is restarting
  request->send(200, "text/html",
    String("<center>\
    <meta http-equiv=\"refresh\" content=\"10;url=/\"/>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\
    <p><strong>Config info loaded</strong></p>\
    <hr />\
    <p><a href=\"/config\">Open configuration page</a></p>\
    <p>Wait for the ESP8266 to restart with the new settings</p>\
    <p>You will be redirected to the home page in 10 seconds.</p>"));
  
  

  // printNetInfo(&_config, "[ESPHelperWeb Post]", true, true);

  _configChanged = true;
}

void ESPHelperWebConfig::setFlashReset(const char* uri){
  createSafeStringFromCharArray(resetURI, _resetURI);
  resetURI.clear();
  resetURI.print(uri);
  _resetSet = true;
}

void ESPHelperWebConfig::handleReset(AsyncWebServerRequest *request){
  //tell the user that the config is loaded in and the module is restarting
  #warning "TODO: Implement flash reset from web config"
  // _server->send(200, "text/plain", String("Resetting SPIFFS and restarting with default values"));

  // LittleFS.format();
  // ESP.restart();
}

void ESPHelperWebConfig::handleNotFound(AsyncWebServerRequest *request){
  request->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
