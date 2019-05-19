#include <iostream>
#include "grblTalk.h"
/*
grblTalk::grblTalk(){

}
*/

//==================================================================================

void grblTalk::setup(int width, int height, int x, int y, float scale){
	
	ofxXmlSettings settings;
	settings.loadFile("settings/globalSettings.xml"); //in app/bin/data/settings

	connectPlotter = settings.getValue("app:connectPlotter", 1, 0);
	port = settings.getValue("app:grbl:port", "/dev/ttyACM0", 0);
	baud = settings.getValue("app:grbl:baud", 115200, 0);
	monitorSerial = settings.getValue("app:grbl:monitorSerial", 0, 0);

	homing = settings.getValue("app:grbl:homing", 1, 0);
	killAlarm = settings.getValue("app:grbl:killAlarm", 0, 0);
	invertY = settings.getValue("app:grbl:invertY", 1, 0);
	feedrate = settings.getValue("app:grbl:feedrate", 5000, 0);
	acceleration = settings.getValue("app:grbl:acceleration", 200, 0);
	
	trackingInterval = settings.getValue("app:livetrackingMaxInterval", 10, 0);
	trackingBufferDist = settings.getValue("app:livetrackingBufferDist", 100, 0);
	plotterBufferDist = settings.getValue("app:grbl:maxBufferDist", 100, 0);

	numScrubSamples = settings.getValue("app:scrubSamples", 50, 0);
	scrubThreshold = settings.getValue("app:scrubThreshold", 500, 0);
	maxScrubDist = settings.getValue("app:maxScrubDist", 100, 0);

	fetchIntervalMin = settings.getValue("app:fetchIntervalMin", 80, 0);
	fetchIntervalMax = settings.getValue("app:fetchIntervalMax", 160, 0);
	fetchWipeDistMax = settings.getValue("app:fetchWipeDistMax", 200, 0);
	fetchWipeDistMin = settings.getValue("app:fetchWipeDistMin", 50, 0);

	exportPath_tracking = settings.getValue("app:exportPath_tracking", "../../../../export_tracking/", 0);	
	exportPath_drawing = settings.getValue("app:exportPath_drawing", "../../../../export_drawing/", 0);		

	pageWidth = width;
	pageHeight = height;
	pageX = x;
	pageY = y;
	pageScale = scale;

	connected = false;

	if(connectPlotter) {

		connected = connectSerial(port, baud, 5000);
		serial.flush();
		while(serial.available() <= 0) {
			getStatus();
		}
		/*
		while(serial.available() > 0) {
			int byte = serial.readByte();
			cout << ofToString(byte);
		}
		*/		
		//ofSleepMillis(1000);
		if(killAlarm) 
			sendMessage("$X");
		else
			serial.writeByte('\n');
	}
	/*
	cout << pageWidth << endl;
	cout << pageHeight << endl;	
	cout << pageX << endl;
	cout << pageY << endl;
	cout << pageScale << endl;	
	*/
	/*
	int w = ofGetWidth();
	int h = ofGetHeight();
	cout << w << endl;
	cout << h << endl;	
	w = ofGetWindowWidth();
	h = ofGetWindowHeight();
	cout << w << endl;
	cout << h << endl;
	*/

	// set state flags
	ready = false;
	gotResponse = false;
	idle = connected ? false : true;
	paused = true;

	updateFreq = 300;
	updateTimer = 0;

	/*
	int n = 100;
	drawList.resize(n);
	drawList[0] = ofVec3f(pageWidth/2, pageHeight/2);
	for(int i=1; i<n; i++) {
		drawList[i] = ofVec3f(drawList[i-1].x + ofRandom(-30,30), drawList[i-1].y + ofRandom(-30,30));
	}
	*/
	
	currentPos = ofVec3f(0,0);
	lineNum = 0;

	// init scrubbing filter
	sampleIndex = 0;
	sampleRange.set(0,0);
	for(int i = 0; i < numScrubSamples; i++) {
		scrubSamples[i].set(0,0);
	}
	fetchInterval = ofRandom(fetchIntervalMin, fetchIntervalMax);

	exportDrawings = 1;
	/*
	queueVertex(0,pageHeight);
	queueVertex(pageWidth,pageHeight);
	queueVertex(pageWidth,0);
	queueVertex(0,0);
	queueVertex(0,pageHeight);
	*/

	/*
	queueVertex(0,2000);
	queueVertex(2000,2000);
	queueVertex(2000,0);
	queueVertex(0,0);
	queueVertex(0,2000);
	*/
	
}

//==================================================================================

void grblTalk::update(float eyeX, float eyeY, bool recording, bool live){
	

	// -------------------------------------------- Recieving & Parsing status updates

	if(connected && connectPlotter) {
		while(serial.available() > 0) {
			char a = serial.readByte();			
			inString += a;
			
			if(a == '\n') {
				
				inString = ofTrim(inString);
				if(monitorSerial)
					cout << inString << endl;

				if (inString.length() > 0){
				
					if (inString == "ok") {
						if(!ready) {
							sendConfig();
							ready = true;
							cout << "ready" << endl;
						}
						gotResponse = true;
					}
			
					if (inString.at(0) == '<') {
						
						inString = inString.substr(1, inString.size()-2); // trim off the chevrons < >
						
						vector<string> status = ofSplitString(inString, "|");
						//cout << status[0] << " " << status[1] << " " << status[2] << endl;
						vector<string> pos = ofSplitString(status[1], ",");
						int i = pos[0].find_first_of(':') + 1;
						pos[0] = pos[0].substr(i,pos[0].size()-i);
						//cout << pos[0] << " " << pos[1] << " " << pos[2] << endl;
						float x = ofToFloat(pos[0]);
						float y = ofToFloat(pos[1]);
						if(invertY) y = pageHeight - y;
						currentPos = ofVec3f(x, y);

						if(status[0] == "Idle") {
							idle = true;

						} else if (status[0] == "Run")  {

							for(int i=2; i<status.size(); i++) { // look through the rest of the status message until we find the line number field
								string num = status[i];
								string code = num.substr(0,2);
								if(code == "Ln") {
									int h = num.find_first_of(':') + 1;
									num = num.substr(h, num.size()-1);
									lineNum = ofToInt(num);
									//cout << "draw line " << lineNum << endl;
									break;
								}
							}							
						}
					}

				}

				inString = "";
			}
		}
	}

	// ----------------------------------------------- Queuing and Sending GCode

	if (connected && gotResponse) {

		// processing queue, wait for "ok" response between sending lines
		if(sendQueue.size() > 0) {
			sendMessage(sendQueue[0]);
			sendQueue.erase(sendQueue.begin());
			gotResponse = false;
		}


		// queuing GCode
		if(!paused) {
			if (drawList.size() > 0) {

				bool send = true;

				// if livetracking, only send gcode up to plotterBufferDist
				float d = 0; // travel distance already in buffer
				if(live) { 

					if(traceList.size() > 0 && lineNum > 0) { // once vertexes have been sent
						// find the total distance yet to be drawn (from lineNum to traceList.size())
						ofVec3f v = traceList[lineNum-1]; // current target vertex
						d = vDist(currentPos, v); // remaining distance of current line
						ofVec3f pv;
						//cout << "buf dist | " << currentPos.x << ", " << currentPos.y << " : " << v.x << ", " << v.y << " : " << d << " | ";
						for(int i=lineNum; i<traceList.size(); i++) {
							pv = v;
							v = traceList[i];
							float d1 = vDist(pv, v);
							d += d1;
							//cout << v.x << ", " << v.y << " : " << d1 << " | ";
						}
						//cout << d << endl;

					} else {
						//ofVec3f v;
						float x1, y1, x2, y2;
						x1 = currentPos.x;
						y1 = currentPos.y;
						if(traceList.size() > 0) {
							//v = traceList[0];
							x2 = traceList[0].x;
							y2 = traceList[0].y;
						} else {
							//v = drawList[0];
							x2 = drawList[0].x;
							y2 = drawList[0].y;

						}
						d = ofDist(x1, y1, x2, y2);
						//cout << "buf dist init: " << currentPos.x << ", " << currentPos.y << " : " << v.x << ", " << v.y << " : " << d << " ? " << vDist(currentPos, v) << endl;
						//cout << "buf dist init: " << x1 << ", " << y1 << " : " << x2 << ", " << y2 << " : " << d << " ? " << ofDist(x1, y1, x2, y2) << endl;
					}

					if(traceList.size() == 0) { // always send first vertex
						send = true;
					} else if(d > plotterBufferDist) {
						//cout << "sending blocked " << paused << endl;
						// otherwise only send if remaining travel distance in buffer
						// is less than plotterBufferDist
						send = false;
					}
				}

				if(send) {

					//cout << "send line " << traceList.size() << " of " << (drawList.size() + traceList.size()) << 	endl;

					sendQueue.push_back(toGcode(drawList[0], traceList.size()+1));
					traceList.push_back(drawList[0]);
					drawList.erase(drawList.begin());
					idle = false;
				}
			}
		}
	}


	// update status
	//if(!idle && !paused) {
	if(!paused) {	
		getStatus();	
	}

	if(recording) {
		recordVertex(eyeX, eyeY);
		//queueVertex(eyeX, eyeY);
		
	} else if(live && !paused) { // if calibrated and livetracking

		float d2 = 0; // dist from current position to last queued vertex
		ofVec3f v;

		if(traceList.size() > 0) {
			v = traceList[traceList.size()-1];
		} else {
			v = currentPos;
		}

		if(drawList.size() > 0) {
			ofVec3f pv;
			for(int i=0; i<drawList.size(); i++) {
				pv = v;
				v = drawList[i];
				d2 += vDist(pv, v);
			}
		}

		if(d2 < trackingBufferDist) { // check if total queued distance is less than bufferDist

			ofVec3f eye = ofVec3f(eyeX, eyeY);
			float d1 = vDist(v, eye);

			if(d1 > trackingInterval) { // if eye position dist from last queued vertex > maxInterval

				float dr = trackingInterval/d1; // ratio of distance we want to total dist
				float dx = eyeX - v.x;
				float dy = eyeY - v.y;	

				float x = v.x + (dr * dx);
				float y = v.y + (dr * dy);

				queueVertex(x, y);
				//cout << drawList.size() << " | " << currentPos.x << " " << currentPos.y << " : " << eyeX << " " << eyeY << " : " << x << " " << y << endl;

			} else {
				queueVertex(eyeX, eyeY);			
			}			
		}

	}

}

//==================================================================================

void grblTalk::draw(){

	ofNoFill();	
	ofSetHexColor(0x00FFFF);
	ofDrawRectangle(0, 0, pageWidth, pageHeight);

	float x, y;

	// plotter position
	ofFill();
	ofSetHexColor(0xFF0000);
	x = currentPos.x;
	y = currentPos.y;
	ofDrawCircle(x, y, 10);
	ofNoFill();	

	// trackList path
	ofSetHexColor(0x00FF00);
	ofBeginShape();
	for(int i=0; i<trackList.size(); i++) {
		x = trackList[i].x;
		y = trackList[i].y;
		ofDrawCircle(x, y, 2);
		ofVertex(x, y);
	}
	ofEndShape();

	// drawList path
	ofSetHexColor(0x00FFFF);
	ofBeginShape();
	if(traceList.size() > 0) {
		int n = traceList.size()-1;
		ofDrawCircle(traceList[n].x, traceList[n].y, 2);
		ofVertex(traceList[n].x, traceList[n].y);		
	}
	for(int i=0; i<drawList.size(); i++) {
		x = drawList[i].x;
		y = drawList[i].y;
		ofDrawCircle(x, y, 2);
		ofVertex(x, y);
	}
	ofEndShape();

	// traceList path
	ofSetHexColor(0x0000FF);
	ofBeginShape();
	for(int i=0; i<traceList.size(); i++) {
		x = traceList[i].x;
		y = traceList[i].y;
		ofDrawCircle(x, y, 2);
		ofVertex(x, y);
	}
	ofEndShape();

}

//--------------------------------------------------------------
void grblTalk::keyPressed(int key){
	//switch (key){		
	//}	
}

//=========================== PATH ==================================


void grblTalk::recordVertex(float x, float y) {

	ofVec3f v = ofVec3f(x,y);
	trackList.push_back(constrain(v));

}

void grblTalk::generatePath() {
	drawList.clear();
	for(int i=0; i<trackList.size(); i++) {
		generateVertex(trackList[i].x, trackList[i].y);
	}
	addParkPos();
}

void grblTalk::generateVertex(float x, float y) {

	// check range of last vertexes to know if to apply scrubbing filter
	scrubSamples[sampleIndex].set(x, y); // add sample to array
	sampleIndex++;
	sampleIndex %= numScrubSamples;
	
	float minX = 999999;
	float minY = 999999;
	float maxX = -999999;
	float maxY = -999999;

	for(int i = 0; i < numScrubSamples; i++) { 
		minX = scrubSamples[i].x < minX ? scrubSamples[i].x : minX;
		minY = scrubSamples[i].y < minY ? scrubSamples[i].y : minY;
		maxX = scrubSamples[i].x > maxX ? scrubSamples[i].x : maxX;
		maxY = scrubSamples[i].y > maxY ? scrubSamples[i].y : maxY;
	}

	sampleRange.set(maxX - minX, maxY - minY);

	//cout << (sampleRange.x + sampleRange.y)/2 << " ";

	float scrubDist =  scrubThreshold - (sampleRange.x + sampleRange.y)/2;
	if(scrubDist > 0) {
		scrubDist *= (float(maxScrubDist) / float(scrubThreshold));
		//cout << scrubDist << endl;
		x += ofRandom(-scrubDist, scrubDist);
		y += ofRandom(-scrubDist, scrubDist);
	}

	//cout << x << " " << y << endl;

	queueVertex(x, y);

	if(drawList.size() % fetchInterval == 0) {
		float w = ofRandom(fetchWipeDistMin, fetchWipeDistMax);
		float dir = ofRandom(0,2);
		if(dir < 1) w *= -1;
		queueVertex(x - w/2, pageHeight);
		queueVertex(x + w/2, pageHeight);
		fetchInterval = ofRandom(fetchIntervalMin, fetchIntervalMax);
	}

}

void grblTalk::queueVertex(float x, float y) {

	ofVec3f v = ofVec3f(x,y);
	drawList.push_back(constrain(v));

}

ofVec3f grblTalk::constrain(ofVec3f v) {

	// constrain within page limits
	if(v.x > pageWidth) v.x = pageWidth;
	if(v.x < 0) v.x = 0;
	if(v.y > pageHeight) v.y = pageHeight;
	if(v.y < 0) v.y = 0;

	return v;
}

float grblTalk::vDist(ofVec3f v1, ofVec3f v2) {
	/*
	float dx = v1.x - v2.x;
	float dy = v1.x - v2.x;
	float d = sqrt(dx*dx + dy*dy);
	return d;
	*/
	return(ofDist(v1.x, v1.y, v2.x, v2.y));
}

void grblTalk::resetPaths() {
	drawList.clear();
	traceList.clear();
	trackList.clear();
}

void grblTalk::addParkPos() {
	//cout << "TRACKED: " << trackList.size() << endl;
	ofVec3f v = drawList[drawList.size()-1];
	drawList.push_back(ofVec3f(v.x, pageHeight));
}


void grblTalk::exportPDF() {
	ofBeginSaveScreenAsPDF(exportPath_tracking + ofGetTimestampString() + ".pdf", true);

	ofBackground(255,255,255);
	ofNoFill();	
	ofSetHexColor(0x000000);
	ofDrawRectangle(0, 0, pageWidth, pageHeight);
	
	float x, y;
	ofBeginShape();
	if(trackList.size() > 0) {
		for(int i=0; i<trackList.size(); i++) {
			x = trackList[i].x;
			y = trackList[i].y;
			ofVertex(x, y);
		}
	}
	ofEndShape();
	ofEndSaveScreenAsPDF();		

	for(int h=0; h<exportDrawings; h++) {
		
		drawList.clear();
		generatePath();

		ofBeginSaveScreenAsPDF(exportPath_drawing + ofGetTimestampString() + ".pdf", true);

		ofBackground(255,255,255);
		ofNoFill();	
		ofSetHexColor(0x000000);
		
		float x, y;
		ofBeginShape();
		if(drawList.size() > 0) {
			for(int i=0; i<drawList.size(); i++) {
				x = drawList[i].x;
				y = drawList[i].y;
				ofVertex(x, y);
			}
		}
		ofEndShape();
		ofEndSaveScreenAsPDF();	
	}
	
	trackList.clear();


}


//=========================== COMMS ==================================



bool grblTalk::connectSerial(string port, int baud, int retryDelay) {

	cout << "connecting to " << port << endl;
	while(!serial.setup(port, baud)) {
		cout << '.';
   	ofSleepMillis(retryDelay);
	}
	connected = true;
	serial.flush();	
	return true;
}


string grblTalk::toGcode(ofVec3f v, uint32_t n) {
	string msg;
	float y = v.y;
	if(invertY) y = pageHeight - y;
	msg = "N" + ofToString(n) + " G0 X" + ofToString(v.x, 2) + " Y" + ofToString(y, 2);
	return msg;
}

void grblTalk::sendMessage(string msg) {
	msg += "\n";
	unsigned char* writeByte = (unsigned char*)msg.c_str();
	if(connectPlotter) {
		serial.writeBytes(writeByte, msg.length());
		//cout << "> " << msg;
	}
}


void grblTalk::getStatus() {
	uint64_t t = ofGetElapsedTimeMillis();
	if(t - updateTimer > updateFreq) {
		if(connected)
			serial.writeByte('?');
		updateTimer = t;
	}
}


void grblTalk::sendConfig() {
	
	// set max feedrate
	sendQueue.push_back("$110=" + ofToString(feedrate)); // X
	sendQueue.push_back("$111=" + ofToString(feedrate)); // Y

	// set acceleration	
	sendQueue.push_back("$120=" + ofToString(acceleration)); // X
	sendQueue.push_back("$121=" + ofToString(acceleration)); // Y

	if(homing) {
		sendQueue.push_back("$H"); // Go Home
		sendQueue.push_back("?"); // update status after homing
	}


}