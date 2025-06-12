/*
ESPHelperWebConfig.h
Copyright (c) 2019 ItKindaWorks All right reserved.
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



#ifndef ESPHELPER_WEBCONFIG_H
#define ESPHELPER_WEBCONFIG_H


#include "ESPHelper.h"
// #include "LittleFS.h"

#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#endif

#ifdef ESP32
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#endif

#include <WiFiClient.h>


static const char ESPHelperWebConfigRoot[] PROGMEM = "<header>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\
<title>System Configuration</title>\
<style body=\"text-align:center;\"font-family:verdana;\"></style>\
</header>\
<body style=\"background-color:#00dfff;\"><font style=\"font-family:verdana;\">\
<center>\
<h3 style=\"text-align:center;\"><span style=\"color:#0a4f75;\"><strong>ESP8266 System Configuration</strong></span></h3>\
<hr />\
<form action=\"%HELPER_PAGE_URI%\" method=\"POST\">\
Device Name:</br>\
<input type=\"text\" name=\"hostname\" size=\"45\" maxlength=\"63\" placeholder=\"Device Hostname  (Required)\" value=\"%HELPER_HOSTNAME%\"></br>\
SSID:</br>\
<input type=\"text\" name=\"ssid\" size=\"45\" maxlength=\"63\" placeholder=\"SSID  (Required)\" value=\"%HELPER_SSID%\"></br>\
SSID Password:</br>\
<input type=\"password\" name=\"netPass\" size=\"45\" maxlength=\"63\" placeholder=\"Network Password (Previous value used if blank)\"></br>\
OTA Password:</br>\
<input type=\"password\" name=\"otaPassword\" size=\"45\" maxlength=\"63\" placeholder=\"OTA Password (Previous value used if blank)\"></br>\
MQTT Host (IP):</br>\
<input type=\"text\" name=\"mqttHost\" size=\"45\" maxlength=\"63\" placeholder=\"MQTT Host\" value=\"%HELPER_MQTT_HOST%\"></br>\
MQTT User:</br>\
<input type=\"text\" name=\"mqttUser\" size=\"45\" maxlength=\"63\" placeholder=\"MQTT Username\" value=\"%HELPER_MQTT_USER%\"></br>\
MQTT Port:</br>\
<input type=\"text\" name=\"mqttPort\" size=\"45\" maxlength=\"63\" placeholder=\"MQTT Port\" value=\"%HELPER_MQTT_PORT%\"></br>\
MQTT Password:</br>\
<input type=\"password\" name=\"mqttPass\" size=\"45\" maxlength=\"63\" placeholder=\"MQTT Password (Previous value used if blank)\"></br>\
<p>Press Submit to update ESP8266 config file</br>\
<input type=\"submit\" value=\"Submit\"></form>\
<p font-size=\"font-size:1.2;\"><a href=/>Go to Device Status Page</a></p>";


class ESPHelperWebConfig{

public:
	ESPHelperWebConfig(int port, const char* URI);  //constructor
#ifdef ESP8266
    ESPHelperWebConfig(ESP8266WebServer *server, const char* URI);
#endif
#ifdef ESP32
    ESPHelperWebConfig(WebServer *server, const char* URI);
#endif

    bool begin(const char* hostname);
    bool begin();

    void useConfig(NetInfo& config);

    void fillConfig(bool preFill = true);

    bool handle();

    NetInfo& getConfig();

    void setFlashReset(const char* uri);


private:
    void handleGet();
    void handlePost();
    void handleNotFound();
    void handleReset();

#ifdef ESP8266
    ESP8266WebServer *_server;
    ESP8266WebServer _localServer;
#endif
#ifdef ESP32
    WebServer *_server;
    WebServer _localServer;
#endif


    const char* _resetURI;
    const char* _pageURI;

    bool _preFill = false;

    bool _resetSet = false;

    NetInfo _config;
    bool _runningLocal = false;
    bool _configChanged = false;


};

#endif
