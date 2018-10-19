
#define LED 3
#define EYE_LEDS_1 5
#define EYE_LEDS_2 6
#define EYE_LEDS_3 9
#define FACE_SENSOR A5
#define FACE_SENSOR_POWER A4
#define LED_DEFAULT 100

String inString = "";

int ledLevel = LED_DEFAULT;
int faceState = 0;
int eyeLedLevel = 0;
int faceThreshold = 800;

int serialInterval = 100;
long serialTimer = 0;

int blinkInterval = 500; // LED blinks while serial is not connected
long blinkTimer = 0;

boolean connected = false;
int connectionTimeout = 5000; // if nothing is received over serial after a while, change state to disconnected.
long connectionTimer;

void setup() {
		
	pinMode(LED, OUTPUT);
	pinMode(EYE_LEDS_1, OUTPUT);
	pinMode(EYE_LEDS_2, OUTPUT);
	pinMode(EYE_LEDS_3, OUTPUT);
	pinMode(FACE_SENSOR, INPUT);
	pinMode(FACE_SENSOR_POWER, OUTPUT);

	serialTimer = millis();
	blinkTimer = millis();
	connectionTimer = millis();

	analogWrite(LED, ledLevel);
	digitalWrite(FACE_SENSOR_POWER, LOW);

	Serial.begin(115200);

	Serial.println("ready");
}


void loop() {

	analogWrite(LED, ledLevel);
	analogWrite(EYE_LEDS_1, 255 - eyeLedLevel);
	analogWrite(EYE_LEDS_2, 255 - eyeLedLevel);
	analogWrite(EYE_LEDS_3, 255 - eyeLedLevel);

	if(connected && millis() - connectionTimer > connectionTimeout) {
		connected = false;
		ledLevel = 0;
		digitalWrite(FACE_SENSOR_POWER, LOW);
	}

	if(!connected && millis() - blinkTimer > blinkInterval) {
	
		ledLevel = abs(ledLevel - LED_DEFAULT); // switches between 0 and LED_DEFAULT.
		blinkTimer = millis();
	
	} else if(connected && millis() - serialTimer > serialInterval) {
		faceState = analogRead(FACE_SENSOR);
		if(faceState < faceThreshold) {
			Serial.println(1);
		} else {
			Serial.println(0);			
		}
		serialTimer = millis();
	}

	while(Serial.available() > 0) {
		char inChar = Serial.read();
		if(inChar == '\n') {
			parseVars(inString);
		} else {
			inString += inChar;
		}
		//connected = true;
		//digitalWrite(FACE_SENSOR_POWER, HIGH);
		connectionTimer = millis();
	}

}


void parseVars(String s) {  

	char cmd = s.charAt(0);
	String varString = s.substring(1, s.length());
	int var = varString.toInt();

	switch(cmd) {

		case 'L':
			ledLevel = var;
			break;

		case 'E':
			eyeLedLevel = var;
			break;

		case 'T':
			faceThreshold = var;
			break;
	}

	switch(cmd) {		
		case 'L':
		case 'E':
		case 'T':
			if(!connected) ledLevel = 0;
			connected = true;
			digitalWrite(FACE_SENSOR_POWER, HIGH);
			break;
	}

	/*
	Serial.print(cmd);
	Serial.print(" = ");
	Serial.println(var);
	*/
	inString = "";

}
