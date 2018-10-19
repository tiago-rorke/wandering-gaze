#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class viewfinderIO {

	public:
		void setup();
		void update();
		void draw();

		// ================================================ //

		// Settings
		bool connectViewfinder;

		// Serial Comms
		ofSerial serial;
		string port;
		int baud;
		bool monitorSerial;
		string 	inString;
		void sendMessage(string msg);
		void sendMessage(string msg, bool monitor);
		// ping arduino to keep connected flag true
		uint16_t pingFreq; 
		uint64_t pingTimer;
		void ping();

		// Machine parameters
		float ledLevel;
		float eyeLevel;
		void setLed(int pwm);
		void setEyeLeds(int pwm);

		// state flags
		bool connected;
		bool gotFace;
		bool ledOn;
		bool pled;
		
};