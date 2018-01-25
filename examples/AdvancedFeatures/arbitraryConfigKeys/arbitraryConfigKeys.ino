/*    
arbitraryConfigKeys.ino
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

//filename and json key to use in the demo
const char* file = "/test.json";
const char* keyName = "count";

void setup(void){
  Serial.begin(115200);

  //load the filesystem and initialize the key to 0 (then close the FS)
  //Note: Although it is not strictly necessary to close the FS it is still a good idea
  //to only have the FS open as long as it is needed and then close it again to prevent
  //possible data corruption. It does make things a bit slower but it's also safer.
  ESPHelperFS::begin();
  ESPHelperFS::addKey(keyName, "0", file);
  ESPHelperFS::end();
}

void loop(void){

  //load the FS and load the config file key
  ESPHelperFS::begin();
  String value = ESPHelperFS::loadKey(keyName, file);

  

  //convert the string to int and increment the counter
  int counter = value.toInt();
  counter++;

  //print out the new value of the counter
  Serial.print("Counter = ");
  Serial.println(counter);

  //convert the key back to a char string for saving
  char dataOut[5];
  sprintf(dataOut, "%d", counter);

  //save the updated key and close the filesystem
  ESPHelperFS::addKey(keyName, dataOut, file);
  ESPHelperFS::end();

  //wait for a second
  delay(1000);
}
