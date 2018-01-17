/*    
ESPHelperWebConfig.h
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
    


#ifndef ESPHELPER_WEBCONFIG_H
#define ESPHELPER_WEBCONFIG_H

#include <ESP8266WebServer.h>
#include "ESPHelper.h"
#include <WiFiClient.h>
#include <ESP8266mDNS.h>


class ESPHelperWebConfig{

public:	
	ESPHelperWebConfig(int port, const char* URI);  //constructor
    ESPHelperWebConfig(ESP8266WebServer *server, const char* URI);

    bool begin(const char* hostname);
    bool begin();

    void fillConfig(netInfo* fillInfo);

    bool handle();

    netInfo getConfig();

    
    
    
private:
    void handleGet();
    void handlePost();
    void handleNotFound();

    ESP8266WebServer *_server;
    ESP8266WebServer _localServer;

    char _newSsid[64];
    char _newNetPass[64];
    char _newOTAPass[64];
    char _newHostname[64];
    char _newMqttHost[64];
    char _newMqttUser[64];
    char _newMqttPass[64];
    int _newMqttPort;

    const char *_pageURI;

    netInfo* _fillData;
    bool _preFill = false;

    netInfo _config;
    bool _configLoaded = false;
    bool _runningLocal = false;

	
};

#endif


