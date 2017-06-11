#pragma once

#define DHTPIN D4	
#define tempFilter 4
#define humidityFilter 4
#define MQTTMinInterval 5
#define ApinTopic "APIN"

byte lastsecond;
byte RefreshCounter;
byte resetcounter;
float rawTempC;
float rawTempF;
float rawHumidity;
float filterTempC;
float filterTempF;
float filterHumidity;
//int mqttcounter;

float l_rawTempC;
float l_rawHumidity;
float l_filterTempC;
float l_filterHumidity;

const long PinModeMask[6] = { 1 + 2 , 4 + 8 , 16 + 32, 64 + 128,256 + 512,1024 + 2048 + 4096 + 8192 };
const byte PinModeShift[6] = { 0,2,4,6,8,10 };
const byte PinModePin[6] = { D5,D6,D7,D8,D3,D4 };
const String Topics[6] = { "pin1", "pin2", "pin3", "pin4", "pin5", "pin6" };
const boolean timeoutpin[6] = { false,false,true,true,false,false };
int timeoutpinTimeValue[6] = { 0,0,0,0,0,0 };

volatile long freq[6] = { 0,0,0,0,0,0 };  //Freq value where enabled
volatile long freqCounter[6] = { 0,0,0,0,0,0 }; //Freq Counter incremented by interrupt
volatile long totaliser[6] = { 0,0,0,0,0,0 }; // totaliser 
int  updateCounter[9] = { 0,0,0,0,0,0,0,0,0 }; //MQTT update interval counter extra item for APIN
#define APINUPDATECOUNTER 6
#define TEMPUPDATECOUNTER 7
#define HUMUPDATECOUNTER 8

int intMode[6] = { CHANGE, CHANGE, CHANGE, CHANGE, CHANGE, CHANGE };


typedef void(*FunctionPointer)();

/*
the io pinmodes are stored as binary maps
for basic io pin
		00 basic input
		01 basic output
		10 Frequency in where support
		11 PWM out

Temp Pin is 
*/

const byte pInput  =B0000; //  basic input
const byte pOutput =B0001; //   basic output
const byte pFreqIn =B0010; //   Frequency in where supported
const byte pPWMOut =B0011; //   PWM out
const byte pDHT11  =B0100; //   DHT 11
const byte pDHT22  =B0110; //   DHT 22
const byte pDallas =B1000; //   Dallas 







