/*    
    wifiRGB.ino
    Copyright (c) 2016 ItKindaWorks All right reserved.
    github.com/ItKindaWorks

    This file is part of wifiRGB

    wifiRGB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    wifiRGB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with wifiRGB.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
	This is an MQTT RGB light controller program for the ESP8266.
	The light can be controlled in one of three ways. You can send
	an HSB update, RGB update, or moodlight mode on/off.
	All updates must be formatted correctly, see below:

	ex RGB string:			"r255,050,000"    	(r = 255, g = 050, b = 000)
	ex HSB string:			"h1.00,0.50,0.01"	(h = 1.00, s = 0.50, b = 0.01)
	ex moodlight string:	"m1"				(moodlight activate)

	This program also posts a status update to the status topic
	which is the lightTopic plus "/status" (ex. if the lightTopic
	is "/home/RGBlight" then the statusTopic would be "home/RGBlight/status")
*/

#include "ESPHelper.h"
#include <HSBColor.h>
#include "Metro.h"

#define TOPIC "/your/mqtt/topic"
#define STATUS TOPIC "/status"
#define NETWORK_HOSTNAME "YOUR OTA HOSTNAME"
#define OTA_PASSWORD "YOUR OTA PASSWORD"

#define RED_PIN 12		
#define GREEN_PIN 13
#define BLUE_PIN 14

typedef struct lightState{
	double hue;
	double saturation;
	double brightness;
	int red;
	int redRate;
	int green;
	int greenRate;
	int blue;
	int blueRate;
	int fadePeriod;
	int updateType;
};

typedef struct timer {
	unsigned long previousTime;
	int interval;
};



enum superModes {SET, MOOD};
enum moodColors{RED, GREEN, BLUE};
enum modes {NORMAL, FADING};
enum updateTypes{HSB, RGB, POWER};


lightState nextState;

int superMode = SET;	//overall mode of the light (moodlight, network controlled, etc)
boolean newCommand = false;


char* lightTopic = TOPIC;
char* statusTopic = STATUS;
char* hostnameStr = NETWORK_HOSTNAME;

const int redPin = RED_PIN;
const int greenPin = GREEN_PIN;
const int bluePin = BLUE_PIN;

char statusString[50];	//string containing the current setting for the light


//set this info for your own network
netInfo homeNet = {.name = "NETWORK NICKNAME", .mqtt = "YOUR MQTT-IP", .ssid = "YOUR SSID", .pass = "YOUR NETWORK PASS"};

ESPHelper myESP(&homeNet);


void setup() {
	//initialize the light as an output and set to LOW (off)
	pinMode(redPin, OUTPUT);
	pinMode(bluePin, OUTPUT);
	pinMode(greenPin, OUTPUT);

	//all off
	digitalWrite(redPin, LOW);	//all off
	digitalWrite(greenPin, LOW);
	digitalWrite(bluePin, LOW);

	delay(1000);

	colorTest();

	//setup ota on esphelper
	myESP.OTA_enable();
	myESP.OTA_setPassword(OTA_PASSWORD);
	myESP.OTA_setHostnameWithVersion(hostnameStr);

	//subscribe to the lighttopic
	myESP.addSubscription(lightTopic);
	myESP.begin();
	myESP.setCallback(callback);
}



void loop(){
	static bool connected = false; //keeps track of connection state to reset from MOOD to SET when network connection is made


	if(myESP.loop() == FULL_CONNECTION){

		//if the light was previously not connected to wifi and mqtt, update the status topic with the light being off
		if(!connected){
			connected = true;	//we have reconnected so now we dont need to flag the setting anymore
			myESP.publish(statusTopic, "h0.00,0.00,0.00	", true);
		}

		lightHandler();
	}

	
	yield();
}


void lightHandler(){
	//new and current lightStates
	static lightState newState;
	static lightState currentState;

	static int currentMoodColor = 0;	//the current moodlight chosen color

	static int isFading = 0;


	//if the super mode is mood light and the light isnt currently fading,
	//then change to a new color and start a new fade
	if(superMode == MOOD && isFading == 0){
		if(currentMoodColor == RED){
			nextState.red = 0;
			nextState.green = 1023;
			nextState.blue = 0;
			nextState.updateType = RGB;
			newCommand = true;
			currentMoodColor = GREEN;

		}
		else if(currentMoodColor == GREEN){
			nextState.red = 0;
			nextState.green = 0;
			nextState.blue = 1023;
			nextState.updateType = RGB;
			newCommand = true;
			currentMoodColor = BLUE;
		}
		else if(currentMoodColor == BLUE){
			nextState.red = 1023;
			nextState.green = 0;
			nextState.blue = 0;
			nextState.updateType = RGB;
			newCommand = true;
			currentMoodColor = RED;
		}
	}

	lightUpdater(&newState, currentState);
	isFading = lightChanger(newState, &currentState);

}


//this function actually changes the light values and does the fading
//returns 1 if fading
//returns 0 if not fading
//returns -1 if the timer doesnt get triggered
int lightChanger(lightState newState, lightState *currentState){
	static Metro changeTimer = Metro(1);

	static int changeMode = NORMAL;		//the current mode (fading or normal)
	static int currentPeriod = 0;		//time since starting the fade
	

	//only allow fade updates every 1ms
	if(changeTimer.check()){

		//check to see if this is there is a new command and set the mode to FADING
		if(newCommand){
			newCommand = false;
			changeMode = FADING;
		}


		
		if(changeMode == FADING){

			//check whether or not a fade is needed - if so update the channel velues
			if((newState.red != currentState->red || newState.blue != currentState->blue || newState.green != currentState->green) || (currentPeriod <= currentState->fadePeriod)){

				if(currentPeriod % newState.redRate == 0){
					if(newState.red > currentState->red){currentState->red++;}
					else if (newState.red < currentState->red){currentState->red--;}
				}

				if(currentPeriod % newState.greenRate == 0){
					if(newState.green > currentState->green){currentState->green++;}
					else if (newState.green < currentState->green){currentState->green--;}
				}

				if(currentPeriod % newState.blueRate == 0){
					if(newState.blue > currentState->blue){currentState->blue++;}
					else if (newState.blue < currentState->blue){currentState->blue--;}
				}

				//write to the analog pins
				analogWrite(redPin, currentState->red);
				analogWrite(greenPin, currentState->green);
				analogWrite(bluePin, currentState->blue);

				//increment the period
				currentPeriod++;
				return 1;	//return 1 on mode being FADING
			}

			//if no fade is needed then reset the period and set the mode to NORMAL
			else{
				currentPeriod = 0;
				changeMode = NORMAL;
				return 0;	//return 0 on mode being NORMAL
			}

		}


		else if (changeMode == NORMAL){
			return 0;	//return 0 on mode being NORMAL
		}
	
	}

	return -1;	//return -1 on timer not set off

}

//calculates new information (color values, fade times, etc) for 
//new color updates
void lightUpdater (lightState *newState, lightState currentState){

	//calculate new vars only if there is a new command
	if (newCommand){

		//determine which kind of update this is
		if(nextState.updateType == HSB){

			//convert from HSB to RGB
			int newRGB[3];
			H2R_HSBtoRGBfloat(nextState.hue, nextState.saturation, nextState.brightness, newRGB);
			newState->red = newRGB[0];
			newState->green = newRGB[1];
			newState->blue = newRGB[2];

			//determine the RGB difference from the current values to new values (how far each channel needs to fade)
			int redDiff = abs(newState->red - currentState.red);
			int greenDiff = abs(newState->green - currentState.green);
			int blueDiff = abs(newState->blue - currentState.blue);

			//calculate the new fade times for each channel (how long to wait between fading up/down)
			if(redDiff > 0){newState->redRate = (nextState.fadePeriod / redDiff);}
			else{newState->redRate = nextState.fadePeriod;}

			if(greenDiff > 0){newState->greenRate = (nextState.fadePeriod / greenDiff);}
			else{newState->greenRate = nextState.fadePeriod;}

			if(blueDiff > 0){newState->blueRate = (nextState.fadePeriod / blueDiff);}
			else{newState->blueRate = nextState.fadePeriod;}

			//set the total time to fade
			newState->fadePeriod = nextState.fadePeriod;

		}
		else if(nextState.updateType == RGB){

			//set new RGB values from update
			newState->red = nextState.red;
			newState->green = nextState.green;
			newState->blue = nextState.blue;

			//determine the RGB difference from the current values to new values (how far each channel needs to fade)
			int redDiff = abs(newState->red - currentState.red);
			int greenDiff = abs(newState->green - currentState.green);
			int blueDiff = abs(newState->blue - currentState.blue);

			//calculate the new fade times for each channel (how long to wait between fading up/down)
			if(redDiff > 0){newState->redRate = (nextState.fadePeriod / redDiff) + 1;}
			else{newState->redRate = nextState.fadePeriod;}

			if(greenDiff > 0){newState->greenRate = (nextState.fadePeriod / greenDiff) + 1;}
			else{newState->greenRate = nextState.fadePeriod;}

			if(blueDiff > 0){newState->blueRate = (nextState.fadePeriod / blueDiff) + 1;}
			else{newState->blueRate = nextState.fadePeriod;}

			//set the total time to fade
			newState->fadePeriod = nextState.fadePeriod;

		}
	}

}



//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {

	//convert topic to string to make it easier to work with
	String topicStr = topic; 

	char newPayload[40];
	memcpy(newPayload, payload, length);
	newPayload[length] = '\0';

	//handle HSB updates
	if(payload[0] == 'h'){
		nextState.hue = atof(&newPayload[1]);
		nextState.saturation = atof(&newPayload[7]);
		nextState.brightness = atof(&newPayload[13]);

		nextState.updateType = HSB;
		nextState.fadePeriod = 2100;
		newCommand = true;
		superMode = SET;

	}

	//handle RGB updates
	else if (payload[0] == 'r'){
		int newRed = atoi(&newPayload[1]);
		int newGreen = atoi(&newPayload[5]);
		int newBlue = atoi(&newPayload[9]);

		nextState.red = newRed;
		nextState.green = newGreen;
		nextState.blue = newBlue;

		nextState.updateType = RGB;
		newCommand = true;
		nextState.fadePeriod = 2100;
		superMode = SET;
	}

	//handle moodlight updates
	else if(payload[0] == 'm'){

		if(payload[1] == '1'){
			superMode = MOOD;
			nextState.fadePeriod = 10000;
			newCommand = true;
		}
		else if(payload[1] == '0'){
			superMode = SET;
			nextState.fadePeriod = 2100;
			newCommand = true;
		}
	}


	//package up status message reply and send it back out to the status topic
	strcpy(statusString, newPayload);
	myESP.publish(statusTopic, statusString, true);
}




void colorTest(){
	digitalWrite(redPin, HIGH);	//red on
	delay(500);
	digitalWrite(redPin, LOW);	//green on
	digitalWrite(greenPin, HIGH);
	delay(500);
	digitalWrite(greenPin, LOW);	//blue on
	digitalWrite(bluePin, HIGH);
	delay(500);

	digitalWrite(redPin, HIGH);	//all on
	digitalWrite(greenPin, HIGH);
	digitalWrite(bluePin, HIGH);
	delay(500);
	digitalWrite(redPin, LOW);	//all off
	digitalWrite(greenPin, LOW);
	digitalWrite(bluePin, LOW);
}


char *ftoa(char *a, double f, int precision)
{
 long p[] = {0,10,100,1000,10000,100000,1000000,10000000,100000000};
 
 char *ret = a;
 long heiltal = (long)f;
 itoa(heiltal, a, 10);
 while (*a != '\0') a++;
 *a++ = '.';
 long desimal = abs((long)((f - heiltal) * p[precision]));
 itoa(desimal, a, 10);
 return ret;
}




float map_double(double x, double in_min, double in_max, double out_min, double out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


