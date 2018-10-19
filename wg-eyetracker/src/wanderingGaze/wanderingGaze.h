#pragma once
#include "ofMain.h"
#include "grblTalk.h"
#include "viewfinderIO.h"

#define MAX_OFFSET_SAMPLES 300 // @ 30 fps = max 10s of samples for calculating offset


class wanderingGaze {
	
	public:
	
		void setup();
		void update(float eyeX, float eyeY, bool newFrame);
		void draw(bool eyeFitted);

		void drawStatus();
		void keyPressed(int key);

		grblTalk plotter;
		viewfinderIO viewfinder;

		ofTrueTypeFont font;
		
		int pageX;
		int pageY;
		int pageWidth;
		int pageHeight;
		float pageScale;

		// state flags
		bool gotFace;
		bool pFace;
		bool gotOffset;
		bool tracking;
		bool recording;
		bool gotRecording;
		bool live;

//		int recordingStartDelay;
//		uint64_t recordingStartTimer;
		int numOffsetSamples;
		ofVec3f offsetSamples[MAX_OFFSET_SAMPLES];
		int sampleIndex;
		ofVec3f sampleRange;
		bool checkSamples();
		void setOffset();
		ofVec3f offsetRef;
		float offsetThreshold; // max range of offset samples allowed to calculate avergage
		int plotterStartDelay;
		uint64_t plotterStartTimer;
		int trackingStartDelay;
		uint64_t trackingStartTimer;

		ofVec3f offset; // offset amount
		ofPoint eye; // eyetracking position with offset applied
		ofPoint eyeOffset; // eyetracking position with offset applied

		bool debugging;

};
