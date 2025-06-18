/*
    sharedTypes.h
    Copyright (c) 2019 ItKindaWorks Inc All right reserved.
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



#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <SafeString.h>


#define VERSION "2-0-4"



//Maximum number of subscriptions that can be auto-subscribed
//feel free to change this if you need more subsciptions
#define MAX_SUBSCRIPTIONS 25

#define DEFAULT_QOS 1;	//at least once - devices are guarantee to get a message.

#define MAX_TOPIC_LENGTH 128


enum connStatus {NO_CONNECTION, BROADCAST, ROAMING, WIFI_ONLY, FULL_CONNECTION};

struct ESPHelperConf {
	char mqttHost[32];
	char mqttUser[16];
	char mqttPass[16];
	int mqttPort;
	char ssid[32];
	char pass[32];
	char otaPassword[16];
	char hostname[32];
	char willTopic[128];
	char willMessage[32];
	int willQoS;
	bool willRetain;
};

class NetInfo {
public:

	/**
	 * @brief Construct a new NetInfo object with local storage
	 * 
	 */
	explicit NetInfo() {
		_conf = new ESPHelperConf();
		_storeLocal = true;
		clear();
	}

	/**
	 * @brief Construct a new NetInfo object with external storage
	 * 
	 * @param externalConf Pointer to an existing ESPHelperConf object
	 */
	explicit NetInfo(ESPHelperConf& externalConf) {
		_conf = &externalConf;
		_storeLocal = false;
	}

	// Deleted copy operations for memory safety
	NetInfo(const NetInfo& other) = delete;
	NetInfo& operator=(const NetInfo& other) = delete;

	/**
	 * @brief Destroy the Net Info object
	 * 
	 */
	~NetInfo() {
		if (_storeLocal && _conf != nullptr) {
			delete _conf;
		}
	}


	

	// ESPHelperConf* getRawConfPtr() const {
	// 	return _conf;
	// }

	/* * @brief Use an external ESPHelperConf object for storage
	 * 
	 * This method allows the NetInfo instance to use an existing ESPHelperConf object
	 * instead of managing its own memory. It is useful for sharing configurations.
	 * 
	 * @param externalConf Pointer to an existing ESPHelperConf object
	 */
	void setExternalMemory(ESPHelperConf* externalConf) {
		if (_storeLocal && _conf != externalConf) {
			delete _conf;
		}
		_conf = externalConf;
		_storeLocal = false;
	}

	/**
	 * @brief Clone the current configuration to another NetInfo object
	 * 
	 * @param target The target NetInfo object to clone to
	 * @param storeLocal If true, the target will store a local copy of the configuration
	 */
	void cloneTo(NetInfo& target, bool storeLocal) const {
		if (storeLocal) {
			target.setToLocalCopy(*_conf);
		} else {
			target.setExternalMemory(_conf);
		}
	}

	// const ESPHelperConf* getConfPointer() const { return _conf; }

	void setMqttHost(const char* val) { safeCopy(_conf->mqttHost, val, sizeof(_conf->mqttHost)); }
	void setMqttUser(const char* val) { safeCopy(_conf->mqttUser, val, sizeof(_conf->mqttUser)); }
	void setMqttPass(const char* val) { safeCopy(_conf->mqttPass, val, sizeof(_conf->mqttPass)); }
	void setSsid(const char* val) { safeCopy(_conf->ssid, val, sizeof(_conf->ssid)); }
	void setPass(const char* val) { safeCopy(_conf->pass, val, sizeof(_conf->pass)); }
	void setOtaPassword(const char* val) { safeCopy(_conf->otaPassword, val, sizeof(_conf->otaPassword)); }
	void setHostname(const char* val) { safeCopy(_conf->hostname, val, sizeof(_conf->hostname)); }
	void setMqttWillTopic(const char* val) { safeCopy(_conf->willTopic, val, sizeof(_conf->willTopic)); }
	void setMqttWillMessage(const char* val) { safeCopy(_conf->willMessage, val, sizeof(_conf->willMessage)); }
	void setMqttPort(int val) { _conf->mqttPort = val; }
	void setMqttWillRetain(bool val) { _conf->willRetain = val; }
	void setMqttWillQoS(int val) { _conf->willQoS = val; }

	const char* getMqttHost() const { return _conf->mqttHost; }
	const char* getMqttUser() const { return _conf->mqttUser; }
	const char* getMqttPass() const { return _conf->mqttPass; }
	const char* getSsid() const { return _conf->ssid; }
	const char* getPass() const { return _conf->pass; }
	const char* getOtaPassword() const { return _conf->otaPassword; }
	const char* getHostname() const { return _conf->hostname; }
	const char* getMqttWillTopic() const { return _conf->willTopic; }
	const char* getMqttWillMessage() const { return _conf->willMessage; }
	int getMqttPort() const { return _conf->mqttPort; }
	int getMqttWillQoS() const { return _conf->willQoS; }
	bool getMqttWillRetain() const { return _conf->willRetain; }

private:

	//denotes whether the ESPHelperConf is stored in external memory or locally within this instance
	bool _storeLocal = true;
	ESPHelperConf* _conf = nullptr;

	/* * @brief Safely copy a string to a destination buffer
	 * 
	 * This method copies a string from source to destination, ensuring that the destination
	 * does not overflow. It also ensures that the destination is null-terminated.
	 * 
	 * @param dest Pointer to the destination buffer
	 * @param src Pointer to the source string
	 * @param maxLen Maximum length of the destination buffer
	 */
	void safeCopy(char* dest, const char* src, size_t maxLen) {
		if (src) {
			strncpy(dest, src, maxLen - 1);
			dest[maxLen - 1] = '\0';
		} else if (dest && maxLen > 0) {
			dest[0] = '\0';
		}
	}

	/* * @brief Clear the configuration data
	 * 
	 * This method resets all fields in the ESPHelperConf structure to their default values.
	 * It is useful for initializing or resetting the configuration.
	 */
	void clear() {
		memset(_conf, 0, sizeof(ESPHelperConf));
		_conf->mqttPort = 1883;
		_conf->willQoS = 1;
		_conf->willRetain = true;
	}


	

	/* * @brief Set the NetInfo to a local copy of the provided ESPHelperConf
	 * 
	 * This method creates a local copy of the provided ESPHelperConf object.
	 * It is useful when you want to ensure that the NetInfo instance has its own copy
	 * of the configuration data.
	 * 
	 * @param conf The ESPHelperConf object to copy from
	 */
	void setToLocalCopy(const ESPHelperConf& conf) {
		if (!_storeLocal) {
			_conf = new ESPHelperConf();
			_storeLocal = true;
		}
		memcpy(_conf, &conf, sizeof(ESPHelperConf));
	}

	
};
// typedef struct NetInfo NetInfo;


struct subscription{
	bool isUsed = false;
	const char* topic;
};
typedef struct subscription subscription;




#endif