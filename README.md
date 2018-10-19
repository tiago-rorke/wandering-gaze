# Wandering Gaze

Interactive installation by Ana Teresa Vicente.  
http://www.anateresavicente.com/index.php/wandering-gaze/

The system comprises:
 - core X-Y 2 axisgantry housed in a slightly oversized picture frame
 - a special viewfinder
 	- IR reflectance sensor to detect the person looking through the viewfinder
 	- IR illumination behind a diffusor in the right eye-cup
 	- 45 degree pane of glass reflects the eye into a ps3eye webcam
 	- small LED attached to the side of the webcam lens
 - a tripod containing a mini pc

This installation was largely made possible thanks to the wonderful [EyeWriter](http://eyewriter.org/) project!  We faced a considerable challenge however, in getting (very roughly) accurate tracking from random people approaching the viewfinder without putting them through the calibration process.  We reached a compromise by creating a kind of auto-calibration where a small red dot would appear in the viewers vision when they approach the viewfinder.  They are told to stare at the dot until it disappears, before proceeding to look at the image.  This position is then used to apply an offset to the resulting tracking data to roughly correct it relative to the position of the viewers eye.


## Sources

The eye-tracking sofware is built using the EyeWriter tool and openFrameworks.  
http://eyewriter.org/

CAD model of installation made with Onshape:  
https://cad.onshape.com/documents/b327fc03410528552058d868/w/0ee9eb11091f6ccb3df238ed/e/d6edd997d3fa7512d0595dc8  
the X-Y gantry is based on a previous plotter design:  
https://cad.onshape.com/documents/9c853be8b394a81c5aa6c314/w/0828b5069770ad60c812175c/e/8f06230b793865605afa23f3  
the motion uses the coreXY design:  
http://corexy.org/  
and the structure uses openbuilds:  
https://openbuilds.com/  

The X-Y motion control is done with GRBL  
https://github.com/gnea/grbl  

Arduino is used in the viewfinder and the plotter:  
https://www.arduino.cc/  
