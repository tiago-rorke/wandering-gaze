
#include "wg-eyetracking.h"
#include "stdio.h"
#include <locale>
//#include <iostream>

//--------------------------------------------------------------
eyetracking::eyetracking(){

}                                                  

//--------------------------------------------------------------
void eyetracking::setup(){
	
	// there is some problem with setting locale using msys2 on windows.
	// is printing many "[ warning] ofUtils: Couldn't create locale  using default, C" messages
	// this fix didn't work (from https://github.com/openframeworks/openFrameworks/issues/5087)
	/*
	#if defined(TARGET_WIN32) && !_MSC_VER //MSYS2 UTF-8 limited support
		setlocale(LC_ALL,"");
		ofLogWarning("ofInit") << "MSYS2 has limited support for UTF-8. using "<< string( setlocale(LC_ALL,NULL) );
	#endif
	*/
	// but changing the log level hides the messages at least, so we can see our other prints
	ofSetLogLevel(OF_LOG_ERROR);

	ofSetVerticalSync(true);
	
	mode = MODE_TRACKING;
	
	gaze.setup();

	TM.setup();
	CM.setup(gaze.pageWidth, gaze.pageHeight, gaze.pageX, gaze.pageY, gaze.pageScale);
   CM.fitter.loadCalibration();

	eyeSmoothed.set(0,0,0);	
	
	bMouseSimulation = false;
	bMouseEyeInputSimulation = false;
}

//--------------------------------------------------------------
void eyetracking::update(){
  	
	TM.update();
	CM.update();

	if (CM.bAutomatic == true && CM.bAmInAutodrive == true && CM.bInAutoRecording){
		if (TM.bGotAnEyeThisFrame() && gaze.gotFace){	
			ofPoint trackedEye = TM.getEyePoint();
			CM.fitter.registerCalibrationInput(trackedEye.x,trackedEye.y);
			CM.inputEnergy = 1;
		}
	}
	 
	if (!bMouseSimulation) {
		if (CM.fitter.bBeenFit){
			ofPoint trackedEye;
			
			if (bMouseEyeInputSimulation) {
				trackedEye.x = mouseX;
				trackedEye.y = mouseY;
			} else {
				trackedEye = TM.getEyePoint();
			}
				
			ofPoint screenPoint = CM.fitter.getCalibratedPoint(trackedEye.x, trackedEye.y);
			eyeSmoothed.x = CM.smoothing * eyeSmoothed.x + (1-CM.smoothing) * screenPoint.x;
			eyeSmoothed.y = CM.smoothing * eyeSmoothed.y + (1-CM.smoothing) * screenPoint.y;
		}
	} else {
		eyeSmoothed.x = mouseX;
		eyeSmoothed.y = mouseY;
	}

	ofPoint pt = eyeSmoothed;
	gaze.update(pt.x, pt.y, TM.IM.bIsFrameNew);
	
}


//--------------------------------------------------------------
void eyetracking::draw(){

	ofBackground(70,70,70);

	ofSetColor(255, 255, 255);
	
	//if (mode == MODE_GAZE) {
		gaze.draw(CM.fitter.bBeenFit);
		drawHelpbox();
	//}

	if (mode == MODE_TRACKING)	{
		TM.draw(gaze.gotFace);
	}

	if (mode == MODE_CALIBRATING)	 {
		CM.draw();
	}

}

void eyetracking::drawHelpbox(){

	ofSetColor(0, 0, 0);
	ofFill();
	int helpx = ofGetWindowWidth()-400;
	int helpy = 50;
	ofRect(helpx, helpy, 250, 80);
	ofSetColor(255, 255, 255);
	helpx += 20;
	helpy += 30;
	ofDrawBitmapString("(return) - change mode", helpx, helpy);
	helpy += 20;
	ofDrawBitmapString("(esc) - exit", helpx, helpy);

}

//--------------------------------------------------------------
void eyetracking::keyPressed(int key){
	
	gaze.keyPressed(key);
	
	switch (key){
			
		case	OF_KEY_RETURN: {
			mode ++;
			mode %= 3; // number of modes;
			} break;
	
		case	'm':
		case	'M': {
			bMouseSimulation = !bMouseSimulation;
			} break;

		case	'q':
		case	'Q': {
			ofFile offset_file (ofToDataPath("settings/offset.txt"), ofFile::WriteOnly);	
			offset_file << eyeSmoothed.x << " " << eyeSmoothed.y << std::endl;
			offset_file.close();
			} break;

		case	'f':
		case	'F': {
			ofToggleFullscreen();
			} break;
	}
	
	if (mode == MODE_CALIBRATING)		CM.keyPressed(key);
	//if (mode == MODE_MOUSE)			mouseScene.keyPressed(key);
}

//--------------------------------------------------------------
void eyetracking::keyReleased(int key){
	 //if (mode == MODE_MOUSE)			mouseScene.keyReleased(key);
}

//--------------------------------------------------------------
void eyetracking::mouseMoved(int x, int y ){
	//if (mode == MODE_MOUSE)			mouseScene.mouseMoved(x, y);
}

//--------------------------------------------------------------
void eyetracking::mouseDragged(int x, int y, int button){
	
	if (mode == MODE_TRACKING)			TM.mouseDragged(x, y, button);
	if (mode == MODE_CALIBRATING)		CM.mouseDragged(x, y, button);
	//if (mode == MODE_TEST)			testScene.mouseDragged(x, y, button);
	//if (mode == MODE_MOUSE)			mouseScene.mouseDragged(x, y, button);
}

//--------------------------------------------------------------
void eyetracking::mousePressed(int x, int y, int button){

	if (mode == MODE_TRACKING)			TM.mousePressed(x, y, button);
	if (mode == MODE_CALIBRATING)		CM.mousePressed(x, y, button);
	//if (mode == MODE_MOUSE)			mouseScene.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void eyetracking::mouseReleased(int x, int y, int button){
	
	if (mode == MODE_TRACKING)			TM.mouseReleased();
	if (mode == MODE_CALIBRATING)		CM.mouseReleased(x,y,button);
	//if (mode == MODE_MOUSE)			mouseScene.mouseReleased(x, y, button);
}

