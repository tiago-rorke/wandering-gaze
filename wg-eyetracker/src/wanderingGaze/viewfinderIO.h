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
		bool connectSerial(string port, int baud, int retryDelay);
		bool monitorSerial;
		string 	inString;
		void sendMessage(string msg);
		void sendMessage(string msg, bool monitor);
		// ping arduino to keep connected flag true
		uint16_t pingFreq; 
		uint64_t pingTimer;
		void ping();

		// Machine parameters
		int ledLevel;
		int eyeLevel;
		int faceThreshold;
		void setLed(int pwm);
		void setEyeLeds(int pwm);
		void setFaceThreshold(int thres);

		// state flags
		bool connected;
		bool gotFace;
		bool ledOn;
		bool pled;
		
};