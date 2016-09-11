/*    
    basicUse.ino
    Copyright (c) 2016 ItKindaWorks All right reserved.
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

#include "ESPHelperCPP.h"
#include "myNetworks.h"

int blinkPin = 2;

ESPHelper myESP(knownNetworks, NETCOUNT, HOME_NET);

void setup() {
	//start the serial line
	Serial.begin(115200);
	delay(500);

	Serial.println("Starting Up, Please Wait...");

	myESP.enableHeartbeat(blinkPin);
	myESP.addSubscription("/test");

	myESP.begin();
	myESP.setCallback(callback);
	
	Serial.println("Initialization Finished.");
}

void loop(){
	myESP.loop();

	yield();
}

void callback(char* topic, uint8_t* payload, unsigned int length) {

}