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


ESPHelperWebConfig::ESPHelperWebConfig(int port, const char* URI) : _localServer(port){
  _server = &_localServer;
  _runningLocal = true;
  _pageURI = URI;
}

#ifdef ESP8266
ESPHelperWebConfig::ESPHelperWebConfig(ESP8266WebServer *server, const char* URI){
  _server = server;
  _runningLocal = false;
  _pageURI = URI;
}
#endif
#ifdef ESP32
ESPHelperWebConfig::ESPHelperWebConfig(WebServer *server, const char* URI){
  _server = server;
  _runningLocal = false;
  _pageURI = URI;
}
#endif

bool ESPHelperWebConfig::begin(const char* _hostname){
  MDNS.begin(_hostname);
  return begin();
}

bool ESPHelperWebConfig::begin(){
  //setup server handlers
  //these handler function definitions use lambdas to pass the funtion... more information can be found here:
  //https://stackoverflow.com/questions/39803135/c-unresolved-overloaded-function-type
  _server->on(_pageURI, HTTP_GET, [this](){handleGet();});        // Call the 'handleRoot' function when a client requests URI "/"
  _server->on(_pageURI, HTTP_POST, [this](){handlePost();}); // Call the 'handleLogin' function when a POST request is made to URI "/login"
  _server->onNotFound([this](){handleNotFound();});           // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

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
	_server->handleClient();
	return _configChanged;
}

NetInfo& ESPHelperWebConfig::getConfig(){
  _configChanged = false;
  return _config;
}





//main config page that allows user to enter in configuration info
void ESPHelperWebConfig::handleGet() {  
  String htmlData = ESPHelperWebConfigRoot;

  if(_preFill){
    htmlData.replace("%HELPER_PAGE_URI%", _pageURI);
    htmlData.replace("%HELPER_HOSTNAME%", _config.getHostname());
    htmlData.replace("%HELPER_SSID%", _config.getSsid());
    htmlData.replace("%HELPER_MQTT_HOST%", _config.getMqttHost());
    htmlData.replace("%HELPER_MQTT_USER%", _config.getMqttUser());
    htmlData.replace("%HELPER_MQTT_PORT%", String(_config.getMqttPort()));
    // Add Will fields
    htmlData.replace("%HELPER_MQTT_WILL_TOPIC%", _config.getMqttWillTopic());
    htmlData.replace("%HELPER_MQTT_WILL_MESSAGE%", _config.getMqttWillMessage());
    htmlData.replace("%HELPER_MQTT_WILL_QOS%", String(_config.getMqttWillQoS()));
    htmlData.replace("%HELPER_MQTT_WILL_RETAIN_0%", _config.getMqttWillRetain() ? "" : "selected");
    htmlData.replace("%HELPER_MQTT_WILL_RETAIN_1%", _config.getMqttWillRetain() ? "selected" : "");
    if(_resetSet){
      htmlData += \
      "<center>"\
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>"\
      "</br></br></br></br></br><form action=\"/reset\" method=\"POST\">"\
      "<input type=\"submit\" value=\"Click Here to Reset ESP Filesystem\"> (WARNING: Deletes all files on device!)</form>";
    }
    _server->send(200, "text/html", htmlData);
  }
  else{
    htmlData.replace("%HELPER_PAGE_URI%", _pageURI);
    htmlData.replace("%HELPER_HOSTNAME%", "");
    htmlData.replace("%HELPER_SSID%", "");
    htmlData.replace("%HELPER_MQTT_HOST%", "");
    htmlData.replace("%HELPER_MQTT_USER%", "");
    htmlData.replace("%HELPER_MQTT_PORT%", "");
    // Add Will fields (empty)
    htmlData.replace("%HELPER_MQTT_WILL_TOPIC%", "");
    htmlData.replace("%HELPER_MQTT_WILL_MESSAGE%", "");
    htmlData.replace("%HELPER_MQTT_WILL_QOS%", "0");
    htmlData.replace("%HELPER_MQTT_WILL_RETAIN_0%", "selected");
    htmlData.replace("%HELPER_MQTT_WILL_RETAIN_1%", "");
    if(_resetSet){
      htmlData += \
      "<center>"\
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>"\
      "</br></br></br></br></br><form action=\"/reset\" method=\"POST\">"\
      "<input type=\"submit\" value=\"Click Here to Reset ESP Filesystem\"> (WARNING: Deletes all files on device!)</form>";
    }
    _server->send(200, "text/html", htmlData);
  }

}

// If a POST request is made to URI /config
void ESPHelperWebConfig::handlePost() {
  // printNetInfo(&_config, "[pre handlePost config:]", true, true);

  //make sure that all the arguments exist and that at least an SSID and hostname have been entered
  if( ! _server->hasArg("ssid") || ! _server->hasArg("netPass")
      || ! _server->hasArg("hostname") || ! _server->hasArg("mqttHost")
      || ! _server->hasArg("mqttUser") || ! _server->hasArg("mqttPass")
      || ! _server->hasArg("mqttPort") || ! _server->hasArg("otaPassword")
      || _server->arg("ssid") == NULL || _server->arg("hostname") == NULL){ // If the POST request doesn't have username and password data

    _server->send(400, "text/plain", "400: Invalid Request - Did you make sure to specify an SSID and Hostname?");  // The request is invalid, so send HTTP status 400
    return;
  }

  //if there is an mqtt user/pass/port entered then there better also be a host!
  if((_server->arg("mqttUser") != NULL || _server->arg("mqttPass") != NULL ||
   _server->arg("mqttPort") != NULL) && _server->arg("mqttHost") == NULL){

   _server->send(400, "text/html",
   String("<center>\
   <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\
   <p><strong>400: Invalid Request - MQTT info specified without host</strong></p>\
   <hr />\
   <p><a href=" + String("config") + ">Open configuration page</a></p>"));
    return;
  }

  //convert the Strings returned by _server->arg to char arrays that can be entered into NetInfo

  if(_server->arg("netPass").length() != 0 or _preFill == false){
    _config.setPass(_server->arg("netPass").c_str());
  }
  if(_server->arg("mqttPass").length() != 0 or _preFill == false){
    _config.setMqttPass(_server->arg("mqttPass").c_str());
  }
  if(_server->arg("otaPassword").length() != 0 or _preFill == false){
    _config.setOtaPassword(_server->arg("otaPassword").c_str());
  }

  _config.setSsid(_server->arg("ssid").c_str());
  _config.setHostname(_server->arg("hostname").c_str());
  _config.setMqttHost(_server->arg("mqttHost").c_str());
  _config.setMqttUser(_server->arg("mqttUser").c_str());

  //the port is special because it doesnt get stored as a string so we take care of that
  if(_server->arg("mqttPort") != NULL){_config.setMqttPort(_server->arg("mqttPort").toInt());}
  else{_config.setMqttPort(1883);}

  // MQTT Will fields
  if (_server->hasArg("mqttWillTopic")) {
    _config.setMqttWillTopic(_server->arg("mqttWillTopic").c_str());
  }
  if (_server->hasArg("mqttWillMessage")) {
    _config.setMqttWillMessage(_server->arg("mqttWillMessage").c_str());
  }
  if (_server->hasArg("mqttWillQos")) {
    _config.setMqttWillQoS(_server->arg("mqttWillQos").toInt());
  }
  if (_server->hasArg("mqttWillRetain")) {
    _config.setMqttWillRetain(_server->arg("mqttWillRetain").toInt() == 1);
  }

  //tell the user that the config is loaded in and the module is restarting
  _server->send(200, "text/html",
    String("<center>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\
    <p><strong>Config info loaded</strong></p>\
    <hr />\
    <p><a href=" + String("config") + ">Open configuration page</a></p>\
    <p>Wait for the ESP8266 to restart with the new settings</p>"));

  // printNetInfo(&_config, "[post handlePost config:]", true, true);

  _configChanged = true;
}

void ESPHelperWebConfig::setFlashReset(const char* uri){
  _resetURI = uri;
  _server->on(_resetURI, HTTP_POST, [this](){handleReset();});
  _resetSet = true;
}

void ESPHelperWebConfig::handleReset(){
  //tell the user that the config is loaded in and the module is restarting
  #warning "TODO: Implement flash reset from web config"
  // _server->send(200, "text/plain", String("Resetting SPIFFS and restarting with default values"));

  // LittleFS.format();
  // ESP.restart();
}

void ESPHelperWebConfig::handleNotFound(){
  _server->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
