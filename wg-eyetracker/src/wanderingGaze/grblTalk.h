#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

#define MAX_SCRUB_SAMPLES 300 // @ 30 fps = max 10s of samples for applying scrubbing filter

class grblTalk {

	public:
		void setup(int width, int height, int x, int y, float scale);
		void update(float eyeX, float eyeY, bool recording, bool live);
		void draw();
		
		void keyPressed(int key);

		// ================================================ //

		// Settings
		bool connectPlotter;
		bool homing;
		bool killAlarm;
		bool invertY;
		bool monitorSerial;
		//void saveSettings();
		//void loadSettings();

		// Serial Comms
		ofSerial serial;
		string port;
		int baud;
		bool connectSerial(string port, int baud, int retryDelay);
		string 	inString;
		void sendMessage(string msg);
		vector<string> sendQueue;
		void getStatus();

		// Machine parameters
		float pageWidth; // max draw area in mm
		float pageHeight;
		float pageX; // page alignment in viewfinder
		float pageY;
		float pageScale;
		int feedrate;
		int acceleration;
		void sendConfig();

		// Paths
		uint32_t lineNum; // current line of gcode the plotter is processing
		ofVec3f currentPos;
		vector<ofVec3f> drawList;
		vector<ofVec3f> traceList;
		string toGcode(ofVec3f v, uint32_t n);
		void queueVertex(float x, float y);
		float vDist(ofVec3f v1, ofVec3f v2);
		ofVec3f constrain(ofVec3f v);
		vector<ofVec3f> trackList; // unaltered eyetracking path for reference
		void recordVertex(float x, float y); // put vertex into trackList, instead of directly into drawList
		void addParkPos();
		void resetPaths();
		void exportPDF();
		int fetchInterval;
		int fetchWipeDistMin;
		int fetchWipeDistMax;
		// for generating "scrubbing" paths
		int numScrubSamples;
		ofVec3f scrubSamples[MAX_SCRUB_SAMPLES];
		int sampleIndex;
		ofVec3f sampleRange;
		int scrubThreshold;
		int maxScrubDist;



		// frequency of status polling ('?' command)
		// grbl wiki reccommends no more than 5Hz max
		uint16_t updateFreq; 
		uint64_t updateTimer;

		// state flags
		bool connected;
		bool ready;
		bool gotResponse;
		bool idle;
		bool paused;

		// live tracking
		float trackingInterval;
		float trackingBufferDist;
		float plotterBufferDist;
		
};