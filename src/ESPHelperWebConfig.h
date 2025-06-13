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
<style>\
body { background-color:#00dfff; font-family:verdana; margin:0; }\
.center-container { display:flex; justify-content:center; align-items:center; min-height:100vh; }\
.config-box { background:#fff; padding:32px 24px; border-radius:12px; box-shadow:0 2px 12px rgba(0,0,0,0.15); min-width:320px; max-width:400px; text-align:left; }\
.config-box h3, .config-box h4 { text-align:center; margin-top:0; }\
.config-box hr { margin:18px 0; }\
.config-box input, .config-box select { width:100%; margin-bottom:10px; padding:6px; border-radius:4px; border:1px solid #ccc; box-sizing:border-box; }\
.config-box input[type=submit] { width:auto; background:#00dfff; color:#fff; border:none; cursor:pointer; padding:8px 18px; border-radius:4px; }\
.config-box input[type=submit]:hover { background:#0a4f75; }\
.config-box p, .config-box a { text-align:center; display:block; margin:10px 0 0 0; }\
</style>\
</header>\
<body>\
<div class=\"center-container\">\
<div class=\"config-box\">\
<h3><span style=\"color:#0a4f75;\"><strong>ESP8266 System Configuration</strong></span></h3>\
<hr />\
<form action=\"%HELPER_PAGE_URI%\" method=\"POST\">\
Device Name:</br>\
<input type=\"text\" name=\"hostname\" size=\"32\" maxlength=\"32\" placeholder=\"Device Hostname  (Required)\" value=\"%HELPER_HOSTNAME%\"></br>\
SSID:</br>\
<input type=\"text\" name=\"ssid\" size=\"32\" maxlength=\"32\" placeholder=\"SSID  (Required)\" value=\"%HELPER_SSID%\"></br>\
SSID Password:</br>\
<input type=\"password\" name=\"netPass\" size=\"32\" maxlength=\"32\" placeholder=\"(Use Stored Value)\"></br>\
OTA Password:</br>\
<input type=\"password\" name=\"otaPassword\" size=\"18\" maxlength=\"16\" placeholder=\"(Use Stored Value)\"></br>\
<hr />\
<h4><span style=\"color:#0a4f75;\">MQTT Settings</span></h4>\
MQTT Host (IP):</br>\
<input type=\"text\" name=\"mqttHost\" size=\"32\" maxlength=\"32\" placeholder=\"MQTT Host\" value=\"%HELPER_MQTT_HOST%\"></br>\
MQTT User:</br>\
<input type=\"text\" name=\"mqttUser\" size=\"16\" maxlength=\"16\" placeholder=\"MQTT Username\" value=\"%HELPER_MQTT_USER%\"></br>\
MQTT Port:</br>\
<input type=\"text\" name=\"mqttPort\" size=\"10\" maxlength=\"10\" placeholder=\"MQTT Port\" value=\"%HELPER_MQTT_PORT%\"></br>\
MQTT Password:</br>\
<input type=\"password\" name=\"mqttPass\" size=\"18\" maxlength=\"16\" placeholder=\"(Use Stored Value)\"></br>\
<hr />\
<h4><span style=\"color:#0a4f75;\">MQTT Will Settings</span></h4>\
Will Topic:</br>\
<input type=\"text\" name=\"mqttWillTopic\" size=\"64\" maxlength=\"128\" placeholder=\"MQTT Will Topic\" value=\"%HELPER_MQTT_WILL_TOPIC%\"></br>\
Will Message:</br>\
<input type=\"text\" name=\"mqttWillMessage\" size=\"32\" maxlength=\"128\" placeholder=\"MQTT Will Message\" value=\"%HELPER_MQTT_WILL_MESSAGE%\"></br>\
Will QoS:</br>\
<input type=\"number\" name=\"mqttWillQos\" min=\"0\" max=\"2\" value=\"%HELPER_MQTT_WILL_QOS%\"></br>\
Will Retain:</br>\
<select name=\"mqttWillRetain\">\
<option value=\"0\" %HELPER_MQTT_WILL_RETAIN_0%>False</option>\
<option value=\"1\" %HELPER_MQTT_WILL_RETAIN_1%>True</option>\
</select></br>\
<p>Press Submit to update ESP8266 config file</p>\
<input type=\"submit\" value=\"Submit\"></form>\
<p><a href=/>Go to Device Status Page</a></p>\
</div></div></body>";


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
