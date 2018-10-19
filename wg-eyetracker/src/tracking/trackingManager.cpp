#include "trackingManager.h"
//#include "buttonTrigger.h"

trackingManager::trackingManager(){

}

trackingManager::~trackingManager(){
	
}

void trackingManager::setup(){
	
	
	
	IM.setup();
	
	guiX = 100;
	guiY = 100;
	imgW = 320;
	imgH = 240;
	
	setupGui();

	//--- set up tracking
	tracker.setup(IM.width, IM.height);
	minBlob		= 10*10;
	maxBlob		= 100*100;
	threshold	= 21;
	
	bFoundEye = false;
	eyePoint.set(0,0,0);
}

void trackingManager::update(){
	//--- update video/camera input
	IM.update();
	
	//--- eye tracking (on new frames)	
	if (IM.bIsFrameNew){
		trackEyes();
	}

	//--- gui
	panel.update();
	updateGui();
}

ofPoint	trackingManager::getEyePoint(){
	return eyePoint;
}

bool trackingManager::bGotAnEyeThisFrame(){
	return bFoundEye;
}

//--------------------------------------------------------------
void trackingManager::trackEyes(){
	tracker.update(IM.grayImage, threshold, minBlob, maxBlob, 0.5f);
	bFoundEye	= tracker.bFoundOne;
	eyePoint	= tracker.getEyePoint();
}


void trackingManager::setupGui(){
	
	panel.setup("cv panel", guiX + 2*imgW + 50, guiY, 300, 450);
	panel.addPanel("image adjustment", 1, false);
	panel.addPanel("edge fixer", 1, false);
	panel.addPanel("blob detection", 1, false);
    //panel.addPanel("button settings", 1, false);
	
	if (IM.mode == INPUT_VIDEO){
		panel.addPanel("video file settings", 1, false);
	} else {
		panel.addPanel("live video settings", 1, false);
	}
	
	//---- img adjust
	panel.setWhichPanel("image adjustment");
	panel.setWhichColumn(0);
	panel.addToggle("flip horizontal ", "B_RIGHT_FLIP_X", false);
	panel.addToggle("flip vertical ", "B_RIGHT_FLIP_Y", false);
	panel.addToggle("rotate clockwise ", "B_RIGHT_ROTCW", false);
	panel.addToggle("rotate counter-clockwise ", "B_RIGHT_ROTCCW", false);
	panel.addToggle("use contrast / bri", "B_USE_CONTRAST", true);
	panel.addSlider("contrast ", "CONTRAST", 0.28f, 0.0, 1.0f, false);
	panel.addSlider("brightness ", "BRIGHTNESS", -0.02f, -1.0, 3.0f, false);
	panel.addToggle("use gamma ", "B_USE_GAMMA", true);
	panel.addSlider("gamma ", "GAMMA", 0.57f, 0.01, 3.0f, false);
	panel.addSlider("threshold ", "THRESHOLD_GAZE", threshold, 0, 255, true);

    //---- blog detect
	panel.setWhichPanel("blob detection");
	panel.addToggle("use dilate", "B_USE_DILATE", true);
	panel.addSlider("dilate num ", "N_DILATIONS", 0, 0, 10, true);
    panel.addSlider("min blob","MIN_BLOB",10*10,0,5000,true);
    panel.addSlider("max blob","MAX_BLOB",100*100,0,50500,true);
	
	//---- tracker edges
	panel.setWhichPanel("edge fixer");
	panel.setWhichColumn(0);
	panel.addSlider("x position ", "EDGE_MASK_X", 320, 0, 640, true);
	panel.addSlider("y position ", "EDGE_MASK_Y", 240, 0, 640, true);
	panel.addSlider("inner radius ", "EDGE_MASK_INNER_RADIUS", 250, 0, 500, true);
	panel.addSlider("outer radius ", "EDGE_MASK_OUTER_RADIUS", 350, 0, 600, true);

	/*
	panel.addSlider("face x position ", "FACE_EDGE_MASK_X", 320, 0, 640, true);
	panel.addSlider("face y position ", "FACE_EDGE_MASK_Y", 240, 0, 640, true);
	panel.addSlider("face inner radius ", "FACE_EDGE_MASK_INNER_RADIUS", 250, 0, 500, true);
	panel.addSlider("face outer radius ", "FACE_EDGE_MASK_OUTER_RADIUS", 350, 0, 600, true);
	*/
	
	if (IM.mode == INPUT_VIDEO){
		panel.setWhichPanel("video file settings");
		// TODO: add theo's video playing things.... [zach]
	} else {
		panel.setWhichPanel("live video settings");
		panel.addToggle("load video settings", "VIDEO_SETTINGS", false);
	}
	
   //---- button settings
	//panel.setWhichPanel("button settings");
   //panel.addSlider("button press time", "BUTTONPRESS_TIME", 5.0f, 0.5f, 30.0f, true);
	
    
    //---- load xml settings
	panel.loadSettings("settings/trackingSettings.xml");
}

void trackingManager::updateGui(){
    	
	tracker.flip(  panel.getValueB("B_RIGHT_FLIP_X"),  panel.getValueB("B_RIGHT_FLIP_Y") );
	tracker.rotate(  panel.getValueB("B_RIGHT_ROTCW"),  panel.getValueB("B_RIGHT_ROTCCW") );
	
	minBlob = panel.getValueI("MIN_BLOB");
	maxBlob = panel.getValueI("MAX_BLOB");
	
	threshold				= panel.getValueI("THRESHOLD_GAZE");
	
	tracker.gamma			= panel.getValueF("GAMMA");
	tracker.bUseGamma		= panel.getValueB("B_USE_GAMMA");

	tracker.contrast		= panel.getValueF("CONTRAST");
	tracker.brightness		= panel.getValueF("BRIGHTNESS");
	tracker.bUseContrast	= panel.getValueB("B_USE_CONTRAST");

	tracker.nDilations		= panel.getValueI("N_DILATIONS");
	tracker.bUseDilate		= panel.getValueB("B_USE_DILATE");

	int oldx				= tracker.edgeMaskStartPos.x;
	int oldy				= tracker.edgeMaskStartPos.y;
	int oldir				= tracker.edgeMaskInnerRadius;
	int oldor				= tracker.edgeMaskOuterRadius;

	tracker.edgeMaskStartPos.x		= panel.getValueI("EDGE_MASK_X");
	tracker.edgeMaskStartPos.y		= panel.getValueI("EDGE_MASK_Y");
	tracker.edgeMaskInnerRadius	= panel.getValueI("EDGE_MASK_INNER_RADIUS");
	tracker.edgeMaskOuterRadius	= panel.getValueI("EDGE_MASK_OUTER_RADIUS");
/*
	int oldfx				= tracker.faceEdgeMaskStartPos.x;
	int oldfy				= tracker.faceEdgeMaskStartPos.y;
	int oldfir				= tracker.faceEdgeMaskInnerRadius;
	int oldfor				= tracker.faceEdgeMaskOuterRadius;

	tracker.faceEdgeMaskStartPos.x	= panel.getValueI("FACE_EDGE_MASK_X");
	tracker.faceEdgeMaskStartPos.y	= panel.getValueI("FACE_EDGE_MASK_Y");
	tracker.faceEdgeMaskInnerRadius	= panel.getValueI("FACE_EDGE_MASK_INNER_RADIUS");
	tracker.faceEdgeMaskOuterRadius	= panel.getValueI("FACE_EDGE_MASK_OUTER_RADIUS");
*/
	if (	oldx	!= tracker.edgeMaskStartPos.x  ||
			oldy	!= tracker.edgeMaskStartPos.y  ||
			oldir	!= tracker.edgeMaskInnerRadius ||
			oldor	!= tracker.edgeMaskOuterRadius
			/*
			|| 
			oldfx	!= tracker.faceEdgeMaskStartPos.x  ||
			oldfy	!= tracker.faceEdgeMaskStartPos.y  ||
			oldfir	!= tracker.faceEdgeMaskInnerRadius ||
			oldfor	!= tracker.faceEdgeMaskOuterRadius	
			*/
			){		

			tracker.calculateEdgePixels();
		
	}

	if (IM.mode != INPUT_VIDEO){
		panel.setWhichPanel("live video settings");
		if (panel.getValueB("VIDEO_SETTINGS") == true){
			
#ifdef TARGET_OSX
	// since macs fuck up bad fullscreen with video settings
			ofSetFullscreen(false);
#endif
			IM.vidGrabber.videoSettings();
			panel.setValueB("VIDEO_SETTINGS", false);
		}
	}
}

void trackingManager::videoSettings(){
	
	
	// TODO: fix this!! [zach]
	//if( !bUseVideoFiles ) ((ofVideoGrabber *)videoSource)->videoSettings();

}

void trackingManager::draw(bool drawTracking){
	
	
	
	ofSetColor(255,255,255);
	
	
	//---------------------------------------------------------------- edge
	if (panel.getSelectedPanelName() == "image adjustment" || panel.getSelectedPanelName() == "live video settings"){
		
		ofSetColor(255,255,255);
	
		tracker.grayImgPreModification.draw(guiX, guiY, imgW, imgH);
		tracker.grayImg.draw(guiX + imgW + 10, guiY, imgW, imgH);				
		tracker.threshImg.draw(guiX, guiY + imgH + 10, imgW, imgH);

		// Attempt at detecting face motion by looking at pixel changes outside of another edge mesk
		//tracker.grayImgFace.draw(guiX, guiY + 2*imgH + 20, imgW, imgH);		
		//tracker.faceMotion.draw(guiX + imgW + 10, guiY + 2*imgH + 20, imgW, imgH);		
		
		ofSetColor(255,255,255);
		ofFill();
		ofRect(guiX + imgW + 10, guiY + imgH + 10, imgW, imgH);
		//ofRect(guiX + imgW + 10, guiY + 2*imgH + 20, imgW, imgH);		
		ofEnableAlphaBlending();
		ofSetColor(255,255,255, 80);
		tracker.grayImgPreModification.draw(guiX + imgW + 10, guiY + imgH + 10, imgW, imgH);	
		
		//tracker.grayImgPreModification.draw(guiX + imgW + 10, guiY + 2*imgH + 20, imgW, imgH);		
		if(drawTracking)
			tracker.eyeTrackedEllipse.draw(guiX + imgW + 10, guiY + imgH + 10, imgW, imgH);

	}
	
	if (panel.getSelectedPanelName() == "edge fixer"){
		ofSetColor(255,255,255);
		tracker.edgeMask.draw(guiX, guiY, 320, 240);
		tracker.grayImg.draw(guiX + imgW + 10, guiY, imgW, imgH);
		
		//tracker.faceEdgeMask.draw(guiX, guiY + imgH + 10, 320, 240);
		//tracker.grayImgFace.draw(guiX + imgW + 10, guiY + imgH + 10, imgW, imgH);
	}

	
	if (panel.getSelectedPanelName() == "blob detection"){
		ofSetColor(255,255,255);
		
		tracker.grayImg.draw(guiX, guiY, 320,240);
		
		tracker.threshImg.draw(guiX + imgW + 10, guiY, imgW, imgH);
		tracker.contourFinder.draw(guiX + imgW + 10, guiY, imgW, imgH);
	}
		
	/*
    if (panel.getSelectedPanelName() == "button settings "){
        buttonPressTime = panel.getValueF("BUTTONPRESS_TIME");
    }
    */

	panel.draw();	
}

//--------------------------------------------------------------
/*
float trackingManager::getButtonPressTime(){
	
	return buttonPressTime;
	
}
*/

//--------------------------------------------------------------
void trackingManager::mouseDragged(int x, int y, int button){
	
	panel.mouseDragged(x, y, button);
	
}

//--------------------------------------------------------------
void trackingManager::mousePressed(int x, int y, int button){
	
	panel.mousePressed(x, y, button);
}

//--------------------------------------------------------------
void trackingManager::mouseReleased(){
	
	panel.mouseReleased();
	
}
