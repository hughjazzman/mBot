#include "MeMCore.h"
#include "Wire.h"

// Not started:
// IR Side Sensors


// to be determined by experiment:
// WAYPTDELAY - delay before decoding waypoint after detection
// SNDTHRESHOLD - value threshold for a sound
// TIMEDELAY - delay for how often position is checked, should be a factor of TIMEGRID
// TIMETURN - time needed for mBot to turn 90 deg
// TIMEGRID - time needed to travel 1 grid

#define WAYPTDELAY 500
#define SNDTHRESHOLD 500
#define TIMEDELAY 500 
#define TIMETURN 1100
#define TIMEGRID 2000 

// Motor setup (movement)
MeDCMotor leftWheel(M1);
MeDCMotor rightWheel(M2);
// Line Follower (Black Strip) setup
MeLineFollower lineFinder(PORT_3); /* Line Finder module can only be connected to PORT_3, PORT_4, PORT_5, PORT_6 of base shield. */
// Color Sensor setup
MeColorSensor colorsensor(PORT_1);
// Ultrasonic Sensor setup
MeUltrasonicSensor ultraSensor(PORT_7); /* Ultrasonic module can ONLY be connected to port 3, 4, 6, 7, 8 of base shield. */
// Sound Sensor setup
MeSoundSensor highSound(PORT_6);
MeSoundSensor lowSound(PORT_5);


// Used in Motor
uint8_t motorSpeed = 100;
// no. of TIMEDELAY cycles to travel 1 grid
int delayGrid = TIMEGRID / TIMEDELAY;

// Used in Line Follower
int lineState;

// Used in Color Sensor
uint8_t colorresult;
uint16_t redvalue = 0, greenvalue = 0, bluevalue = 0, colorvalue = 0;
uint8_t grayscale = 0;
uint16_t colorvalues[5];
long systime = 0, colorcode = 0;

// Used in Ultrasonic Sensor
double frontDistance;

// Used in Sound Sensor
uint8_t highStrength;
uint8_t lowStrength;

// Used in Waypoint Check
int waypoint; // true if at waypoint, false if not


void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	colorsensor.SensorInit();
}

void loop() {
	// put your main code here, to run repeatedly:
	frontDistance = ultraSensor.distanceCm(); // Distance to wall in front
	lineState = lineFinder.readSensors(); // Detection black strip below

	// To add: IR side sensors - check distance and readjust accordingly

	waypoint = checkWaypoint(lineState); // Presence of black strip

	// If waypoint detected, decode it
	if (waypoint) {
		delay(WAYPTDELAY); // Delay before start
		getColours(colorvalues); // Get colours above
		colorresult = colorvalues[0]; // Get preset colour result
		highStrength = highSound.strength(); // Get high f sound strength
		lowStrength = lowSound.strength(); // Get low f sound strength

		// If color waypoint
		if (colorvalues[0] != BLACK)
			colorWaypoint(colorresult);
		// If sound waypoint
		else if (highStrength > SNDTHRESHOLD || lowStrength > SNDTHRESHOLD)
			soundWaypoint(highStrength, lowStrength);
		// If finished
		else
			finish();
	}
	else {
		moveForward();
	}

}

// Functions for movement
// forward
// forwardGrid
// turnRight
// turnLeft
// doubleRight
// doubleLeft
// uTurn
// readjust
void forward() {
	leftWheel.run(motorSpeed);
	rightWheel.run(-motorSpeed);
	delay(TIMEDELAY);
	leftWheel.stop();
	rightWheel.stop();
}

void forwardGrid() {
	for (int i = 0; i < delayGrid;i++) {
		forward();
	}
}

void turnRight() {
	leftWheel.run(motorSpeed);
	rightWheel.run(motorSpeed);
	delay(TIMETURN);
	leftWheel.stop();
	rightWheel.stop();
}

void turnLeft() {
	leftWheel.run(-motorSpeed);
	rightWheel.run(-motorSpeed);
	delay(TIMETURN);
	leftWheel.stop();
	rightWheel.stop();
}

void doubleRight() {
	turnRight();
	forwardGrid();
	turnRight();
}

void doubleLeft() {
	turnLeft();
	forwardGrid();
	turnLeft();
}

void uTurn() {
	turnRight();
	turnRight();
}

void readjust() {}

// Function retrieves colour from sensor
void colorPrint() {
	colorresult = colorsensor.Returnresult();
	redvalue = colorsensor.ReturnRedData();
	greenvalue = colorsensor.ReturnGreenData();
	bluevalue = colorsensor.ReturnBlueData();
	colorvalue = colorsensor.ReturnColorData();
	colorcode = colorsensor.ReturnColorCode();//RGB24code
	grayscale = colorsensor.ReturnGrayscale();

	Serial.print("R:");
	Serial.print(redvalue);
	Serial.print("\t");
	Serial.print("G:");
	Serial.print(greenvalue);
	Serial.print("\t");
	Serial.print("B:");
	Serial.print(bluevalue);
	Serial.print("\t");
	Serial.print("C:");
	Serial.print(colorvalue);
	Serial.print("\t");
	Serial.print("color:");
	switch (colorresult)
	{
	case BLACK:
		Serial.print("BLACK");
		break;
	case BLUE:
		Serial.print("BLUE");
		break;
	case YELLOW:
		Serial.print("YELLOW");
		break;
	case GREEN:
		Serial.print("GREEN");
		break;
	case RED:
		Serial.print("RED");
		break;
	case WHITE:
		Serial.print("WHITE");
		break;
	default:
		break;
	}
	Serial.print("\t");
	Serial.print("code:");
	Serial.print(colorcode, HEX);
	Serial.print("\t");
	Serial.print("grayscale:");
	Serial.println(grayscale);
}


// Function for waypoint checking using Line Sensor
// returns true if black strip
// returns false if no black strip
int checkWaypoint(int lineState) {
	return lineState == S1_IN_S2_IN;
}

// Function for getting colours from Colour Sensor
void getColours(uint16_t colorvalues[]) {
	colorvalues[0] = colorsensor.Returnresult();
	colorvalues[1] = colorsensor.ReturnRedData();
	colorvalues[2] = colorsensor.ReturnGreenData();
	colorvalues[3] = colorsensor.ReturnBlueData();
	colorvalues[4] = colorsensor.ReturnColorData();
}

void colorWaypoint(uint8_t colorresult) {
	// To do
}

void soundWaypoint(uint8_t highStrength, uint8_t lowStrength) {
	// To do
}

void finish() {
	// To do
}

void moveForward() {
	// To do
	// uses side IR sensors
}