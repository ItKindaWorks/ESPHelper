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

#ifdef ESP8266
#include <ESP8266mDNS.h>
#endif

#ifdef ESP32
#include <ESPmDNS.h>
#include <WiFi.h>
#endif

#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>
#include "SafeString.h"

#include "config_html.h"



class ESPHelperWebConfig{

public:
	ESPHelperWebConfig(int port, const char* URI);  //constructor
    ESPHelperWebConfig(AsyncWebServer *server, const char* URI);
    ~ESPHelperWebConfig();  //destructor

    bool begin(const char* hostname);
    bool begin();

    void useConfig(NetInfo& config);

    void fillConfig(bool preFill = true);

    bool handle();

    NetInfo& getConfig();

    void setFlashReset(const char* uri);


private:
    String handleGet(const String& var);
    void handlePost(AsyncWebServerRequest *request);
    void handleNotFound(AsyncWebServerRequest *request);
    void handleReset(AsyncWebServerRequest *request);

    AsyncWebServer *_server;
    
    char _resetURI[64];
    char _pageURI[64];

    bool _preFill = false;

    bool _resetSet = false;

    NetInfo _config;
    bool _runningLocal = false;
    bool _configChanged = false;


};

#endif
