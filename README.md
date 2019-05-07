# ESPHelper
A library to make using WiFi and MQTT on ESPXX platforms easy.

I wrote this library to help with my own home automation projects. It handles connecting (and reconnecting 
when the connection is lost) for both a wifi network and MQTT broker. When reconnecting it automatically 
resubscribes to subscribed topics on an MQTT broker. It is also capable of automatically hopping between
different network/MQTT setups (I wrote this is mostly so that I could move from my home network to work
and have my prototypes be able to connect to whatever environment was most convinient)

The library also features the ability to use the ArduinoOTA system for OTA updates. There are a number of wrapper 
methods for enabling/disabling OTA and changing the OTA hostname and password.

Please take a look at the examples included with this library to get an idea of how it works. 

Note:
-----
This library does requre the use of these libraries (so make sure they're installed as well!):
* [Metro](https://www.pjrc.com/teensy/td_libs_Metro.html)
* [pubsubclient](https://github.com/knolleary/pubsubclient)
* [ArduinoJson 6.x](https://github.com/bblanchon/ArduinoJson)

 In addition to those libraries, make sure that you have the ESP core files installed for your platform.
 * [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
 * [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)

Useful Methods:
---------------

* bool begin(); //must be called (once) to start the system

* int loop();  //must be called as often as possible to maintain connections and run the various subsystems


* bool subscribe(char* topic);  //subscribe to a given MQTT topic (will NOT auto re-subscribe on connection lost)

* bool addSubscription(char* topic);  //add a topic to the subscription list (will auto re-subscribe on connection lost)

* bool removeSubscription(char* topic); //remove a topic from the subscription list and unsubscribe

* void publish(char* topic, char* payload); //publish a given MQTT message to a given topic

* bool setCallback(MQTT_CALLBACK_SIGNATURE);  //set the callback for MQTT (must be called after begin() method)


* void updateNetwork(); //manually disconnect and reconnecting to network/mqtt using current values (generally called after setting new network values)

* String getIP(); //get the current IP of the ESP module


* void setHopping(bool canHop); //enable/disable hopping between networks in a net list


* void OTA_enable();  //enable the OTA subsystem

* void OTA_disable(); //disable the OTA subsystem

* void OTA_begin();   //start the OTA subsystem

* void OTA_setPassword(char* pass); //set a password for OTA updates

* void OTA_setHostname(char* hostname); //give a hostname to the device for OTA identification

ToDo:
-----

* Integrate Last Will into config files and config page
* Implement callback for lost WiFi connection

