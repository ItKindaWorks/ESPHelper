# ESPHelper

ESPHelper is a C++ library designed to simplify WiFi and MQTT connectivity for ESP8266 and ESP32 platforms. It manages WiFi and MQTT connections, automatic reconnection, topic subscriptions, and supports OTA (Over-The-Air) updates, making it ideal for IoT and home automation projects.

## Features

- **Automatic WiFi and MQTT Connection Management:** Handles connecting, reconnecting, and resubscribing to MQTT topics.
- **MQTT Topic Subscription Management:** Add, remove, and auto-resubscribe to topics.
- **OTA Updates:** Easily enable/disable OTA, set OTA password and hostname.
- **Broadcast Mode:** Create an access point for configuration or OTA when no WiFi is available.
- **Web Configuration:** Optional web interface for device configuration ([`ESPHelperWebConfig`](src/ESPHelperWebConfig.h)).
- **Callback Support:** Set custom callbacks for WiFi connection, WiFi loss, and MQTT messages.
- **Secure MQTT:** Supports SSL/TLS connections to MQTT brokers.

## Requirements

Make sure you have these libraries installed:
* [Metro](https://github.com/ItKindaWorks/ESPHelper_Metro)
* [PubSubClient](https://github.com/knolleary/pubsubclient) (Current Supported Version - 2.8)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) (Current Supported Version - 7.0.4)
* [SafeString](https://github.com/PowerBroker2/SafeString) (Current Supported Version - 4.1.30)

 In addition to those libraries, make sure that you have the ESP core files installed for your platform.
 * [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)
 * [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)

## Getting Started

See the [examples/GettingStarted](examples/GettingStarted/) folder for usage examples.

### Basic Usage

```cpp
#include "ESPHelper.h"

ESPHelper helper;

void setup() {
    helper.setSSID("yourSSID");
    helper.setPASS("yourPassword");
    helper.setMQTTIP("mqtt.example.com");
    helper.begin();
}

void loop() {
    helper.loop();
}
```

Useful Methods:
---------------

* *bool begin();*
	Initialize the system (must be called once).

* *int loop();*
    must be called as often as possible to maintain connections and run the various subsystems

* *bool subscribe(char\* topic);*
    subscribe to a given MQTT topic (will NOT auto re-subscribe on connection lost)

* *bool addSubscription(char\* topic);*
    add a topic to the subscription list (will auto re-subscribe on connection lost)

* *bool removeSubscription(char\* topic);*
    remove a topic from the subscription list and unsubscribe

* *void publish(char\* topic, char\* payload);*
    publish a given MQTT message to a given topic

### ToDo

* Implement callback for lost WiFi connection

