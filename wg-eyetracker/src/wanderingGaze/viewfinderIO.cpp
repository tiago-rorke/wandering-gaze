#include "viewfinderIO.h"
#include <iostream>


//--------------------------------------------------------------
void viewfinderIO::setup() {

	ofxXmlSettings settings;
	settings.loadFile("settings/globalSettings.xml"); //in app/bin/data/settings

	connectViewfinder = settings.getValue("app:connectViewfinder", 1, 0);
	port = settings.getValue("app:viewfinder:port", "/dev/ttyACM0", 0);
	baud = settings.getValue("app:viewfinder:baud", 115200, 0);
	monitorSerial = settings.getValue("app:viewfinder:monitorSerial", 0, 0);

	ledLevel = settings.getValue("app:viewfinder:ledBrightness", 1, 0);
	eyeLevel = settings.getValue("app:viewfinder:eyeBrightness", 255, 0);

	gotFace = false;
	ledOn = false;
	pled = false;

	if(connectViewfinder) {
		connected = serial.setup(port, baud);
		serial.flush();
		while(serial.available() <= 0);
		setEyeLeds(eyeLevel);
		setLed(0);
	}

}

//--------------------------------------------------------------
void viewfinderIO::update() {

	if(connected && connectViewfinder) {

		while(serial.available() > 0) {
			char a = serial.readByte();			
			inString += a;
			
			if(a == '\n') {
				
				if (inString.length() > 0) {
					inString = ofTrim(inString);
				}

				if (inString.length() > 0) {
					int a = (int)inString.at(0) - 48; // char to int
					gotFace = a > 0 ? true : false;
				}
				if(monitorSerial)
					cout << inString << endl;
				inString = "";
			}
		}

		if(ledOn != pled) {
			if(ledOn) {
				setLed(ledLevel);
			} else {
				setLed(0);
			}
			pled = ledOn;
		}

		ping();
		
		/*
		if(ledOn && !pled) {
			setLed(ledLevel);
			pled = true;
		}

		if(!ledOn && pled) {
			setLed(0);
			pled = false;
		}
		*/

	}
}

//--------------------------------------------------------------
void viewfinderIO::draw() {

}

//=========================== COMMS ==================================

void viewfinderIO::sendMessage(string msg) {
	sendMessage(msg, true);
}

void viewfinderIO::sendMessage(string msg, bool monitor) {
	msg += "\n";
	unsigned char* writeByte = (unsigned char*)msg.c_str();
	if(connected && connectViewfinder) {
		serial.writeBytes(writeByte, msg.length());
		if(monitor)
			cout << "^ " << msg;
	}
}

void viewfinderIO::setLed(int pwm) {
	sendMessage("L" + ofToString(pwm));
}

void viewfinderIO::setEyeLeds(int pwm) {	
	sendMessage("E" + ofToString(pwm));
}

void viewfinderIO::ping() {
	uint64_t t = ofGetElapsedTimeMillis();
	if(t - pingTimer > pingFreq) {
		sendMessage("?", false);
		pingTimer = t;
	}
}
