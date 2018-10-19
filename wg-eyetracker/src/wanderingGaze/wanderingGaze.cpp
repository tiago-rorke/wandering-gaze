#include <iostream>
#include "wanderingGaze.h"


void wanderingGaze::setup() {

	ofxXmlSettings settings;
	settings.loadFile("settings/globalSettings.xml");
	font.loadFont("fonts/HelveticaNeueMed.ttf", 14);

	pageX = settings.getValue("app:pageX", 890, 0);
	pageY = settings.getValue("app:pageY", 190, 0);	
	int scaledPageWidth = settings.getValue("app:scaledPageWidth", 895, 0);
	pageWidth = settings.getValue("app:pageWidth", 1035, 0);
	pageHeight = settings.getValue("app:pageHeight", 750, 0);

	pageScale = (float)scaledPageWidth / (float)pageWidth;

	plotter.setup(pageWidth, pageHeight, pageX, pageY, pageScale);
	viewfinder.setup();

	recording = false;
	gotRecording = false;

	for(int i = 0; i < MAX_OFFSET_SAMPLES; i++) {
		offsetSamples[i].set(0,0);
	}
	numOffsetSamples = settings.getValue("app:offset:numSamples", 150, 0);
	offsetThreshold = settings.getValue("app:offset:threshold", 10.0, 0);
	float x = settings.getValue("app:offset:refX", 150, 0);
	float y = settings.getValue("app:offset:refY", 150, 0);
	x /= pageScale;
	y /= pageScale;
	offsetRef.set(x,y);
	offset.set(0,0);
	sampleIndex = 0;
	sampleRange.set(0,0);

	eye.set(0,0);
	eyeOffset.set(0,0);

/*
	recordingStartDelay = settings.getValue("app:recordingStartDelay", 4000, 0);
	recordingStartTimer = 0;
*/

	plotterStartDelay = settings.getValue("app:plotterStartDelay", 2000, 0);
	plotterStartTimer = 0;

	trackingStartDelay = settings.getValue("app:trackingStartDelay", 1000, 0);
	trackingStartTimer = 0;

	live = false;
	gotFace = false;
	pFace = false;
	gotOffset = false;
	tracking = false;
	
	debugging = false;

}


void wanderingGaze::update(float eyeX, float eyeY, bool newFrame) {

	eye.set(eyeX,eyeY);
	eyeOffset.x = eye.x + offset.x;
	eyeOffset.y = eye.y + offset.y;

	if(!debugging) {
		plotter.update(eyeOffset.x, eyeOffset.y, recording, live);
	} else {
		plotter.update(eyeOffset.x, eyeOffset.y, false, false);
	}
	
	viewfinder.update();
	gotFace = viewfinder.gotFace;
	
	uint64_t t = ofGetElapsedTimeMillis();

	// --------------------- calibration for new faces ------------------- //

	if(!debugging) {

 		// when a face appears
		if(gotFace && !pFace) {
			gotOffset = false; // reset offset
			offset.set(0,0);
			for(int i = 0; i < numOffsetSamples; i++) {
				offsetSamples[i].set(0,0);
			}
			sampleIndex = 0;
			sampleRange.set(0,0);
			viewfinder.ledOn = true; // switch on led			
			//recordingStartTimer = t;
			pFace = true;
		}

		// recording samples for calibration
		if(gotFace && !gotOffset && newFrame) {
			offsetSamples[sampleIndex].set(eyeX, eyeY); // add sample to array
			sampleIndex++;
			sampleIndex %= numOffsetSamples;
			if(checkSamples()) { // check to see if we have enough good samples yet
				setOffset(); // if so then get the average to set the offset
				gotOffset = true;
				trackingStartTimer = t;
				plotter.resetPaths();
				plotter.lineNum = 0;
				viewfinder.ledOn = false; // switch off led
			}
		}

		if(trackingStartTimer > 0 && t - trackingStartTimer > trackingStartDelay) {
			tracking = true;
			trackingStartTimer = 0;
		}

	}

	// ----------------------- recording/playback mode ------------------- //


	if(!live && !debugging) {

		// once the offset has been calibrated
		//if(recordingStartTimer > 0 && t - recordingStartTimer > recordingStartDelay) {
		if(gotFace && gotOffset && tracking) {
			recording = true;
			//recordingStartTimer = 0;
		}

		// when the face disappears
		if(!gotFace && pFace) {
			recording = false; // stop recording			
			tracking = false;
			//recordingStartTimer = 0;
			plotterStartTimer = t;
			if(plotter.drawList.size() > 0) {
				plotter.addParkPos();
				plotter.exportPDF();
			}
			pFace = false;
			viewfinder.ledOn = false; // switch off led
			//for(int i = 0; i < numOffsetSamples; i++) { 
				//cout << i << " " << offsetSamples[i].x << " " << offsetSamples[i].y << endl;
			//}
			//cout << sampleRange.x << " " << sampleRange.y << endl;
		}

		if(plotterStartTimer > 0 && t - plotterStartTimer > plotterStartDelay) {
			plotter.paused = false;
			plotterStartTimer = 0;
		}

		if (plotter.drawList.size() == 0 && plotter.idle) {
			//cout << sampleRange.x << " " << sampleRange.y << endl;
			plotter.paused = true;
		}

	}

	// ------------------------------ live mode -------------------------- //

	if(live && !debugging) {

		// once the offset has been calibrated
		if(gotFace && gotOffset && tracking) {
			plotter.paused = false;
			pFace = true;
		}

		// when the face disappears
		if(!gotFace && pFace) {
			plotter.paused = true;	
			tracking = false;
			plotter.resetPaths();
			pFace = false;
			viewfinder.ledOn = false; // switch off led
		}

	}

	fflush(stdout);

}


bool wanderingGaze::checkSamples() {

	float minX = 999999;
	float minY = 999999;
	float maxX = -999999;
	float maxY = -999999;

	for(int i = 0; i < numOffsetSamples; i++) { 
		minX = offsetSamples[i].x < minX ? offsetSamples[i].x : minX;
		minY = offsetSamples[i].y < minY ? offsetSamples[i].y : minY;
		maxX = offsetSamples[i].x > maxX ? offsetSamples[i].x : maxX;
		maxY = offsetSamples[i].y > maxY ? offsetSamples[i].y : maxY;
	}

	sampleRange.set(maxX - minX, maxY - minY);
	//cout << "range: " << sampleRange.x << " " << sampleRange.y << endl;

	bool check = sampleRange.x > offsetThreshold || sampleRange.y > offsetThreshold ? false : true;
	return check;
}


void wanderingGaze::setOffset() {
	
	double avgX = 0;
	double avgY = 0;

	for(int i = 0; i < numOffsetSamples; i++) { 
		avgX += offsetSamples[i].x;
		avgY += offsetSamples[i].y;
	}
	avgX /= numOffsetSamples;
	avgY /= numOffsetSamples;
	avgX = offsetRef.x - avgX;
	avgY = offsetRef.y - avgY;
	offset.set(avgX, avgY);

}


void wanderingGaze::draw(bool eyeFitted) {

	if(debugging)
		ofBackground(70,70,110);
	else if(recording)
		ofBackground(110,70,70);
	else if(live)
		ofBackground(70,110,110);

	ofFill();
	
	ofPushMatrix();
	ofTranslate(pageX, pageY);
	ofScale(pageScale, pageScale, pageScale);

	if(viewfinder.gotFace && eyeFitted) {
		
		if(!gotOffset) {
			ofSetColor(255,0,255,30);
			ofEllipse(offsetRef.x, offsetRef.y, sampleRange.x, sampleRange.y);
			ofSetColor(255, 0, 255);
		}
		else if(!tracking) {
			ofSetColor(0,255,0,80);
		} else {
			ofSetColor(0,255,0,120);
		}
		ofCircle(eyeOffset.x, eyeOffset.y, 20);

		if(debugging) {
			ofSetColor(255, 0, 255);
			ofCircle(eye.x, eye.y, 20);
		}
	}

	if (viewfinder.ledOn) {
		ofSetColor(255, 0, 0, 70);
		ofCircle(offsetRef.x, offsetRef.y, 20);
	}
	
	plotter.draw();

	ofPopMatrix();

	drawStatus();

}

void wanderingGaze::drawStatus(){

	int statusX = 100;
	int statusY = ofGetWindowHeight() - 400;
	int linespacing = 25;

	if(plotter.connected) {
		ofSetColor(0, 255, 255);
		font.drawString("plotter connected", statusX, statusY);
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("plotter disconnected", statusX, statusY);	
	}

	statusX += 20;
	statusY += linespacing;
	if(!plotter.paused) {
		ofSetColor(0, 255, 255);
		font.drawString("plotter enabled", statusX, statusY);
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("plotter paused", statusX, statusY);	
	}

	statusX -= 20;
	statusY += linespacing;
	if(viewfinder.connected) {
		ofSetColor(0, 255, 255);
		font.drawString("viewfinder connected", statusX, statusY);
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("viewfinder disconnected", statusX, statusY);	
	}

	statusX += 20;
	statusY += linespacing;
	if(viewfinder.gotFace) {
		ofSetColor(0, 255, 255);
		font.drawString("got a face", statusX, statusY);
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("no face", statusX, statusY);	
	}

	statusY += linespacing;
	if(gotOffset) {
		ofSetColor(0, 255, 255);
		font.drawString("offset calibrated", statusX, statusY);
	} else if(gotFace) {
		ofSetColor(0, 255, 255);
		font.drawString("calibrating offset", statusX, statusY);
		//font.drawString(ofToString(sampleRange.x), statusX + 200, statusY);
		//font.drawString(ofToString(sampleRange.y), statusX + 300, statusY);
		//font.drawString(ofToString(sampleIndex), statusX + 400, statusY);
		ofPushMatrix();
		ofTranslate(pageX, pageY);
		ofScale(pageScale, pageScale, pageScale);
		font.drawString(ofToString(sampleRange.x), offsetRef.x, offsetRef.y + 20);
		font.drawString(ofToString(sampleRange.y), offsetRef.x + 20, offsetRef.y);
		ofPopMatrix();
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("no offset", statusX, statusY);	
	}

	statusY += linespacing;
	if(tracking) {
		ofSetColor(0, 255, 255);
		font.drawString("tracking", statusX, statusY);
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("not tracking", statusX, statusY);	
	}

	statusY += linespacing;
	if(recording) {
		ofSetColor(0, 255, 255);
		font.drawString("recording tracking...", statusX, statusY);
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("not recording", statusX, statusY);	
	}

	statusY += linespacing;
	if(live) {
		ofSetColor(0, 255, 255);
		font.drawString("live tracking", statusX, statusY);
	} else {
		ofSetColor(255, 0, 255);
		font.drawString("live tracking disabled", statusX, statusY);	
	}	


}

void wanderingGaze::keyPressed(int key){

	switch (key){
		
		case 'd':
		case 'D':
			debugging = !debugging;
			if(debugging) {
				plotter.resetPaths();
				recording = false;
				live = false;
				plotterStartTimer = 0;
				//recordingStartTimer = 0;
			}
			break;	

		case 'c':
		case 'C':
			plotter.resetPaths();
			break;	

		case 'f':
		case 'F':
			if(!viewfinder.connectViewfinder) {
				gotFace = !gotFace;

				if(gotFace)
					cout << "got a face" << endl;
				else
					cout << "no face" << endl;		
			}
			break;	

		case 'r':
		case 'R':
			recording = !recording;			
			if(recording) {
				live = false;
				plotter.resetPaths();
			}
			if(recording)
				cout << "recording start" << endl;
			else
				cout << "recording stop" << endl;
			break;

		case 'l':
		case 'L':
			live = !live;
			if (live) {
				recording = false;
				plotter.drawList.clear();	
				plotter.traceList.clear();
				plotter.trackList.clear();	
			}
			if(live)
				cout << "live plotter tracking start" << endl;
			else
				cout << "live plotter tracking stop" << endl;
			break;

		case 'p':
		case 'P':
			plotter.paused = !plotter.paused;
			if(!plotter.paused && recording) {
				recording = false;
			}		
			if(plotter.paused) 
				cout << "plotter paused" << endl;
			else
				cout << "plotter resumed" << endl;
			break;

		case 'o':
		case 'O':
			viewfinder.ledOn = !viewfinder.ledOn;	
			if(viewfinder.ledOn) 
				cout << "toggle LED on" << endl;
			else
				cout << "toggle LED off" << endl;
			break;
			break;
	}

	//plotter.keyPressed(key);

}