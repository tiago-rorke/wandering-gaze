#ifndef _WG_EYETRACKING
#define _WG_EYETRACKING


#include "ofMain.h"
#include "wg-eyetracking.h"
#include "trackingManager.h"	
#include "calibrationManager.h"
#include "wanderingGaze.h"
#include "viewfinderIO.h"


enum{
	
	MODE_TRACKING,
	MODE_CALIBRATING,
	MODE_GAZE

};


class eyetracking : public ofBaseApp {

	public:

		eyetracking();
		void setup();
		void update();
		void draw();
		void drawHelpbox();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		  
		bool bMouseSimulation;
		bool bMouseEyeInputSimulation;

		int pageX;
		int pageY;
		int scaledPageWidth;
		int pageWidth;
		int pageHeight;
				
		ofPoint eyeSmoothed;

		int mode; 

		trackingManager			TM;
		calibrationManager		CM;
		wanderingGaze				gaze;

		//------ drawing
		void drawHelp();
		float buttonSensitivity;

};

#endif
