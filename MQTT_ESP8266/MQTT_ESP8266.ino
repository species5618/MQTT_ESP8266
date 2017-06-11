/*
 Name:		MQTT_ESP8266.ino
 Created:	6/11/2017 6:03:32 PM
 Author:	Lee
*/


#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <DHT_U.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include "FS.h"
#include <WiFiClient.h>
#include <Time.h>
#include <NtpClientLib.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "FSWebServerLib.h"
#include <Hash.h>
#include "global.h"





// #define DISABLEOUT
#define RELEASE  // Comment to enable debug output
#define DBG_OUTPUT_PORT Serial

#ifndef RELEASE
#define DEBUGLOG(...) DBG_OUTPUT_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif


DHT dht(DHTPIN, DHT11);
#define ONE_WIRE_BUS DHTPIN
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature dallas(&oneWire);

long lastinputstate[6] = { 0,0,0,0,0,0 };
// long lastinputstatet[6] = { 0,0,0,0,0,0 };
int lastA0 = -1;
//bool coldstart = true;

bool TimeSync = false;

WiFiClient espClient;
PubSubClient client(espClient);


//Freq Managment, lasy coding style for speed
void rpm0()     //This is the function that the interupt calls 
{
	freqCounter[0]++;  //This function measures the rising and falling edge of the hall effect sensors signal frequency
	totaliser[0]++;  //counter
}
void rpm1()     //This is the function that the interupt calls 
{
	freqCounter[1]++;  //This function measures the rising and falling edge of the hall effect sensors signal frequency
	totaliser[1]++;  //counter
}
void rpm2()     //This is the function that the interupt calls 
{
	freqCounter[2]++;  //This function measures the rising and falling edge of the hall effect sensors signal frequency
	totaliser[2]++;  //counter
}
void rpm3()     //This is the function that the interupt calls 
{
	freqCounter[3]++;  //This function measures the rising and falling edge of the hall effect sensors signal frequency
	totaliser[3]++;  //counter
}
void rpm4()     //This is the function that the interupt calls 
{
	freqCounter[4]++;  //This function measures the rising and falling edge of the hall effect sensors signal frequency
	totaliser[4]++;  //counter
}
void rpm5()     //This is the function that the interupt calls 
{
	freqCounter[5]++;  //This function measures the rising and falling edge of the hall effect sensors signal frequency
	totaliser[5]++;  //counter
}
void Fcount()
{
	cli();            //Disable interrupts
	freq[0] = freqCounter[0];
	freqCounter[0] = 0;
	freq[1] = freqCounter[1];
	freqCounter[1] = 0;
	freq[2] = freqCounter[2];
	freqCounter[2] = 0;
	freq[3] = freqCounter[3];
	freqCounter[3] = 0;
	freq[4] = freqCounter[4];
	freqCounter[4] = 0;
	freq[5] = freqCounter[5];
	freqCounter[5] = 0;
	sei();            //Enables interrupts
}


FunctionPointer rpmList[6] = { rpm0,rpm1, rpm2, rpm3, rpm4, rpm5 };


void callback(char* topic, byte* payload, unsigned int length) {
	String message = "";
	for (int i = 0; i < length; i++) {
		DEBUGLOG((char)payload[i]);
		DEBUGLOG("\r\n");
		message += (char)payload[i];
	}
	//Serial.print(topic);
	//Serial.print(":");
	//Serial.println(message);
	String messsagePart2 = "";

	for (size_t i = 0; i < (sizeof(PinModeMask) / sizeof(long)); i++)
	{
		//		Serial.println(String(i));
		if (((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pOutput) // is output pin
		{
			int x = message.indexOf(":");
			if (x >= 0)
			{
				messsagePart2 = message.substring(x + 1);
				message = message.substring(0, x);
			}
			String topictest = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i];
			if (topictest.equals(String(topic)))
			{
				//String FT = NTP.getDateStr() + " " + NTP.getTimeStr();
				if ((char)message[0] == '1' || (char)message[0] == 'T' || message.substring(0, 2) == "ON")
				{
#ifndef DISABLEOUT
					digitalWrite(PinModePin[i], HIGH);
#endif
					client.publish((topictest + "_C").c_str(), ("ON:" + NTP.getDateStr() + " " + NTP.getTimeStr()).c_str());
					if (messsagePart2.length()>0 && IsNumeric(messsagePart2)) // later test for split command, and over ride default time out
					{
						timeoutpinTimeValue[i] = messsagePart2.toInt();
						//Serial.print("Set pin "+ String(i) +" time out ");
						//Serial.println(timeoutpinTimeValue[i]);
					}
					else if (i == 2)
					{
						timeoutpinTimeValue[i] = ESPHTTPServer._config.pin3t;
						//Serial.print("Set pin3 time out ");
						//Serial.println(timeoutpinTimeValue[i]);
					}
					else if (i == 3)
					{
						timeoutpinTimeValue[i] = ESPHTTPServer._config.pin4t;
						//Serial.print("Set pin4 time out ");
						//Serial.println(timeoutpinTimeValue[i]);

					}
					if (timeoutpinTimeValue[i] > 0)
					{
						updateCounter[i] = timeoutpinTimeValue[i];
						lastinputstate[i] = 1;

					}

				}
				else
				{
					digitalWrite(PinModePin[i], LOW);
					client.publish((topictest + "_C").c_str(), ("OFF:" + NTP.getDateStr() + " " + NTP.getTimeStr()).c_str());
				}

			}


		}
		if (((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pPWMOut) // is output pin
		{
			analogWriteFreq(ESPHTTPServer._config.PWMFreq);
			String topictest = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i];
			if (topictest.equals(String(topic)))
			{
#ifndef DISABLEOUT				
				analogWrite(PinModePin[i], message.toInt());
#endif
			}
		}

		if (((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pFreqIn) // jut simple input
		{
			String topictest = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i] + "_T";

			if (topictest.equals(String(topic)))
			{
				totaliser[i] = atol(message.c_str());
			}
		}
	}




}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		DEBUGLOG("Attempting MQTT connection...");
		// Attempt to connect
		if (client.connect(ESPHTTPServer._config.ClientName.c_str(), ESPHTTPServer._config.MQTTUser.c_str(), ESPHTTPServer._config.MQTTpassword.c_str(),
			("/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/LWT").c_str(), 1, true, "DOWN")) {
			DEBUGLOG("connected");
			// Once connected, publish an announcement...
			//client.publish("outTopic", "hello world");
			//// ... and resubscribe
			//client.subscribe("inTopic");
			client.unsubscribe("#");
			client.publish(("/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/LWT").c_str(), "UP", 1);
		}
		else {
			DEBUGLOG("failed, rc=");
			DEBUGLOG(client.state());
			DEBUGLOG(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}



	for (size_t i = 0; i < (sizeof(PinModeMask) / 4); i++)
	{
		//		Serial.println(String(i));
		if (
			(((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pOutput) //subscribe to MQTT for output pins 
			||
			(((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pPWMOut) //subscribe to MQTT for PWM output pins 
			)  //is output pin
		{
			pinMode(PinModePin[i], OUTPUT);
			if (ESPHTTPServer._config.DeviceMode & MQTTEnable == MQTTEnable)
			{
				//subscribe to MQTT
				String topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i];
				client.subscribe(topic.c_str(), 1);
			}
		}
		else
		{
			pinMode(PinModePin[i], INPUT);
		}

		if (((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pFreqIn) // jut simple input
		{
			//subscribe to MQTT for totlaiser reset 
			String topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i] + "_TR";
			client.subscribe(topic.c_str(), 1);

		}
	}



}

void setup() {

	rawTempC = -99;
	rawTempF = -99;
	rawHumidity = -99;
	filterTempC = -99;
	filterTempF = -99;
	filterHumidity = -99;


	// WiFi is started inside library
	SPIFFS.begin(); // Not really needed, checked inside library and started if needed
	ESPHTTPServer.begin(&SPIFFS);
	/* add setup code here */

	if (((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDHT11)  //DHT11
	{
		//		pinMode(DHTPIN, INPUT_PULLUP);
		dht.begin(DHT11);
		//DBG_OUTPUT_PORT.print("11n");

	}
	else if (((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDallas)  //DHT22
	{
		//pinMode(DHTPIN, INPUT_PULLUP);
		//DBG_OUTPUT_PORT.print("22n");

		dht.begin(DHT22);
	}
	else if (true)
	{
		dallas.begin();
	}

	if (ESPHTTPServer._config.DeviceMode & MQTTEnable == MQTTEnable)
	{
		client.setServer(ESPHTTPServer._config.MQTTHost.c_str(), ESPHTTPServer._config.MQTTPort);
		client.setCallback(callback);
	}
	for (int i = 0; i < sizeof(PinModeMask) / sizeof(long); i++)
	{
		//check inputs
		if (((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pFreqIn) // jut simple input

		{
			attachInterrupt(PinModePin[i], rpmList[i], intMode[i]);
			//Serial.println("F A" + String(i));
		}
	}


#ifdef DISABLEOUT
	Serial.println("Relays disabled");
#endif

	pinMode(A0, INPUT);
	reconnect();

}

void loop() {
	/* add main program code here */

	// DO NOT REMOVE. Attend OTA update from Arduino IDE
	ESPHTTPServer.handle();
	if (!TimeSync && NTP.getLastNTPSync() >0)
	{
		TimeSync = true;
		if (ESPHTTPServer._config.DeviceMode & MQTTEnable == MQTTEnable)
		{
			client.publish(("/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/Boot").c_str(),
				(NTP.getTimeStr() + " " + NTP.getDateStr()).c_str());
		}

	}



	if (ESPHTTPServer.WiFiStatus() == 3)
	{
		if (!client.connected()) {
			reconnect();
			Serial.println("mqtt Connect Client");


		}
		client.loop();

		if (second() != lastsecond)
		{// do refresh
		 //catch F-in and copy counter to values, shoudl be 1000ms interupd, but not simple on esp286
			Fcount();

			for (int i = 0; i < sizeof(updateCounter) / sizeof(int); i++)
			{
				if (updateCounter[i] > 0)
				{
					updateCounter[i]--;
					//Serial.println(String(i) + " " + updateCounter[i]);
				}
			}

			lastsecond = second();
			if (second() % 2 == 0)
			{
				DEBUGLOG(NTP.getUptimeString());

				if (second() % 4 == 0)
				{
					float t = GetTempC();
					if (!isnan(t))
					{
						if (rawTempC == -99)
						{
							filterTempC = t;
						}
						else
						{
							filterTempC = filterTempC + (t - filterTempC) / tempFilter;
							filterTempF = (filterTempC - 32) / 1.8;
						}
						rawTempC = t;
						rawTempF = (rawTempC - 32) / 1.8;
					}
					// Serial.println(String(tempC) + " C");
				}
				else
				{
					float h = GetHum();
					if (!isnan(h))
					{
						if (rawHumidity == -99)
						{
							filterHumidity = h;
						}
						else
						{
							filterHumidity = filterHumidity + (h - filterHumidity) / humidityFilter;
						}
						rawHumidity = h;


					}
					//Serial.println(String(Humidity) +  " %");
				}

			}

			//if ((++mqttcounter >  ESPHTTPServer._config.RefreshInterval || coldstart) && ESPHTTPServer._config.DeviceMode & MQTTEnable == MQTTEnable)
			if (ESPHTTPServer._config.DeviceMode & MQTTEnable == MQTTEnable)
			{
				//mqttcounter = 0;
				//coldstart = false;
				//check value for MQTT publish
				for (int i = 0; i < sizeof(PinModeMask) / sizeof(long); i++)
				{
					//check inputs
					if (((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pInput) // jut simple input
					{
						int t = digitalRead(PinModePin[i]);
						if (lastinputstate[i] != t && updateCounter[i] == 0)
						{
							updateCounter[i] = ESPHTTPServer._config.RefreshInterval;
							lastinputstate[i] = t;
							// mqqt publish
							String topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i];
							client.publish(topic.c_str(), String(t).c_str(), true);
							//						Serial.println(topic);

						}
					}
					if ((((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pOutput) && timeoutpin[i] && (updateCounter[i] == 0 && (lastinputstate[i] == 1))) // output time out
					{
						Serial.println("@");
						lastinputstate[i] = 0;
						String topictest = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i];
						digitalWrite(PinModePin[i], LOW);
						client.publish((topictest + "_C").c_str(), ("OFF:" + NTP.getDateStr() + " " + NTP.getTimeStr()).c_str(), true);
						client.publish((topictest).c_str(), "0", true);
					}

					if (((ESPHTTPServer._config.PinModes & PinModeMask[i]) >> PinModeShift[i]) == pFreqIn) // jut simple input
					{
						cli();            //Disable interrupts
						int t = freq[i];
						int t1 = totaliser[i];
						sei();

						if (lastinputstate[i] != (t + t1) && updateCounter[i] == 0)
						{
							updateCounter[i] = ESPHTTPServer._config.RefreshInterval;
							lastinputstate[i] = t + t1;
							// mqqt publish
							String topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + Topics[i];
							client.publish(topic.c_str(), String(t).c_str(), true);
							topic += "_T";
							client.publish(topic.c_str(), String(t1).c_str(), true);

						}
					}




				}
				if ((ESPHTTPServer._config.DeviceMode & ModePinA0) == ModePinA0)
				{
					int A0 = analogRead(A0);
					if (lastA0 != A0 && updateCounter[APINUPDATECOUNTER] == 0)
					{
						updateCounter[APINUPDATECOUNTER] = ESPHTTPServer._config.RefreshInterval;
						lastA0 = A0;
						String topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + ApinTopic;
						client.publish(topic.c_str(), String(A0).c_str(), 1);
					}
				}
				//float l_rawTempC;
				//float l_rawHumidity;
				//float l_filterTempC;
				if (((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDHT11
					|| ((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDHT22)
					//DHT**
				{
					if ((l_rawTempC != rawTempC || l_filterTempC != filterTempC) && updateCounter[TEMPUPDATECOUNTER] == 0)
					{
						l_rawTempC = rawTempC;
						String topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + "rawTempC";
						client.publish(topic.c_str(), String(l_rawTempC).c_str(), 1);

						l_filterTempC = filterTempC;
						topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + "filterTempC";
						client.publish(topic.c_str(), String(l_filterTempC).c_str(), 1);
					}
					if ((l_rawHumidity != rawHumidity || l_filterHumidity != filterHumidity) && updateCounter[HUMUPDATECOUNTER] == 0)
					{
						l_filterHumidity = filterHumidity;
						String topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + "filterHumidity";
						client.publish(topic.c_str(), String(filterHumidity).c_str(), 1);

						l_rawHumidity = rawHumidity;
						topic = "/" + (String)ESPHTTPServer._config.MQTTTopic + "/" + ESPHTTPServer._config.ClientName + "/" + "rawHumidity";
						client.publish(topic.c_str(), String(rawHumidity).c_str(), 1);
					}
				}
			}

		}

	}
	else
	{
		if (client.connected())
		{
			Serial.println("Disconnect Client");
			Serial.println(espClient.connected());
			client.disconnect();
		}
	}
}

float GetTempC()
{
	if (((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDHT11
		|| ((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDHT22)  //DHT**
	{
		return dht.readTemperature();
	}
	else if (((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDallas)
	{
		return dallas.getTempCByIndex(0);
	}
	return -99;
}

float GetHum()
{
	if (((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDHT11
		|| ((ESPHTTPServer._config.PinModes & PinModeMask[5]) >> PinModeShift[5]) == pDHT22)  //DHT**
	{
		return dht.readHumidity();
	}
	return -99;
}


boolean IsNumeric(String str) {
	unsigned int stringLength = str.length();

	if (stringLength == 0) {
		return false;
	}

	boolean seenDecimal = false;

	for (unsigned int i = 0; i < stringLength; ++i) {
		if (isDigit(str.charAt(i))) {
			continue;
		}

		if (str.charAt(i) == '.') {
			if (seenDecimal) {
				return false;
			}
			seenDecimal = true;
			continue;
		}
		return false;
	}
	return true;
}