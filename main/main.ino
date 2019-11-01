
#include <MeDCMotor.h>

#include <MeLineFollower.h>

#include <MeMCore.h>

#include <MeRGBLed.h>

#include <MeUltrasonicSensor.h>


#include "MeMCore.h"
#include "Wire.h"

// Done:
// general movement - values calibrated

// Half Done:
// colour sensor (?) - to recalibrate

// Not done:
// IR side sensors (?) -not tested (Walter)
// sound sensors
// finish


// determined by experiment:
// WAYPTDELAY - delay before decoding waypoint after detection
// TIMEDELAY - delay for how often position is checked, should be a factor of TIMEGRID
// TIMETURN - time needed for mBot to turn 90 deg
// TIMEGRID - time needed to travel 1 grid

// to be determined by experiment:
// REDARR,GREARR,YELARR,PURARR,BLUARR - RGB Values of each colour stored in arrays
// SNDTHRESHOLD - value threshold for a sound
// FRNTTHRESHOLD - distance (cm) in front of mBot there is a wall for waypoint
// SIDETHRESHOLD - voltage (V) corresponding to closeness to side walls
// COLOURTHRESHOLD - max deviation from calibrated colour values

#define WAYPTDELAY 100
#define SNDTHRESHOLD 500
#define TIMEDELAY 20
#define TIMETURN 1100
#define TIMEGRID 2250
#define FRNTTHRESHOLD 5
#define SIDETHRESHOLD 0.5

// Infrared side sensor pins
#define IRL A1 //left
#define IRR A2 //right

// Sound sensor pins
#define SNDLOW A3 //low
#define SNDHI A0 //high

/* COLOR SENSOR CONSTANTS*/
// Define time delay before the next RGB colour turns ON to allow LDR to stabilize
#define RGBWait 200 //in milliseconds 

// Define time delay before taking another LDR reading
#define LDRWait 10 //in milliseconds 

#define LDR A6   //LDR sensor pin at A6

#define MAXLED 255 // max value of LED RGB values

#define COLOURTHRESHOLD 10 //max deviation from calibrated colour values

// Calibrated values for each colour used in colour waypoint test
// Values retrieved from colourcal.ino file
#define REDARR {,,}
#define GREARR {,,}
#define YELARR {,,}
#define PURARR {,,}
#define BLUARR {,,}


/*MCORE OBJECTS*/

// Motor setup (movement)
MeDCMotor leftWheel(M1);
MeDCMotor rightWheel(M2);
// Line Follower (Black Strip) setup
MeLineFollower lineFinder(PORT_2);
// Ultrasonic Sensor setup
MeUltrasonicSensor ultraSensor(PORT_1);
// Color Sensor setup - confirmed
MeRGBLed led(7); // pin 7 is the RGB LED (WHY??)
// Sound Sensor setup
//MeSoundSensor highSound(PORT_6);
//MeSoundSensor lowSound(PORT_5);

/* VARIABLES */

// Used in Motor
uint8_t motorSpeed = 100;
// no. of TIMEDELAY cycles to travel 1 grid
int delayGrid = TIMEGRID / TIMEDELAY;

// Used in IR Sensor
// value comes from IR sensor in voltage
float rightDist = 0;
float leftDist = 0;

// Used in Line Follower
int lineState; // whether its sensors detect line below in 1 or both of them


/* COLOUR VARIABLES*/

//placeholders for colour detected
int red = 0;
int green = 0;
int blue = 0;

//floats to hold colour arrays
float colourArray[] = {0, 0, 0};
float whiteArray[] = {0, 0, 0};
float blackArray[] = {0, 0, 0};
float greyDiff[] = {0, 0, 0};
int allColourArray[5][3] = {0, 0, 0}; // red,green,yellow,purple,lightblue
int finalColVal[5][3] = {REDARR,GREARR,YELARR,PURARR,BLUARR};

char colourStr[3][5] = {"R = ", "G = ", "B = "};

//resulting colour at waypoints (calibration done before start)
int colourRes = 0;

/*ULTRASONIC SENSOR VARIABLES*/
// Used in Ultrasonic Sensor
double frontDistance;

// Used in Sound Sensor
uint8_t highStrength;
uint8_t lowStrength;

// Used in Waypoint Check
int waypoint; // true if at waypoint, false if not


void setup() {
  // put your setup code here, to run once:
  pinMode(IRL, INPUT);
  pinMode(IRR, INPUT);
  pinMode(SNDLOW, INPUT);
  pinMode(SNDHI, INPUT);
  //pinMode(13, OUTPUT); // RGBLed
  Serial.begin(9600);

  //colour calibration first
  // to do: save values in array
  setBalance();
}

void loop() {

  /* To test Movement*/

  //forward();
  // turnRight();
  //  turnLeft();
  //uTurn();
  //doubleRight();
  //  doubleLeft();
  //  forwardGrid();

  
  // put your main code here, to run repeatedly:
  frontDistance = ultraSensor.distanceCm(); // Distance to wall in front
  lineState = lineFinder.readSensors(); // Detection black strip below

  // Side wall "distances" (in V)
  rightDist = analogRead(IRR);
  leftDist = analogRead(IRL);


  /* To test LineFinder*/

  /*
    waypoint = checkWaypoint(lineState); // Presence of black strip
    if (lineState != S1_OUT_S2_OUT){
      stopMove();
      Serial.println("stop");
      turnRight();
    }
    else {
      forward();
      Serial.println("forward");
    }
  */
  /*To test IR sensor*/

  /*To test colour detection */
  /*
  colourRes = getColour();
  colourWaypoint(colourRes);
  */
  /* To test sound */


    /* Actual logic here */
    // If waypoint detected, decode it
    if (waypoint) {
      stopMove();
  	  delay(WAYPTDELAY); // Delay before start
  	  colourRes = getColour(); // Get preset colour result
  	  //highStrength = highSound.strength(); // Get high f sound strength
  	  //lowStrength = lowSound.strength(); // Get low f sound strength

  	  // If color waypoint
  	  if (colourRes >= 0)
  		  colourWaypoint(colourRes);
  	  // If sound waypoint
  	  //else if (highStrength > SNDTHRESHOLD || lowStrength > SNDTHRESHOLD)
  		  //soundWaypoint(highStrength, lowStrength);
  	  // If finished
  	  else
  		  finish();
    }

    // If no waypoint
    
    if (tooClose(rightDist, leftDist)) {
  	  reAdjust(rightDist, leftDist);
    }
    else {
  	  forward();
    }
}

/*MOVEMENT FUNCTIONS*/
// forward
// stopMove
// forwardGrid
// turnRight
// turnLeft
// doubleRight
// doubleLeft
// uTurn

// Below - to be checked with IR sensor
// reAdjust
// reAdjustRight
// reAdjustLeft

void forward() {
  leftWheel.run(-motorSpeed);
  rightWheel.run(motorSpeed);
  delay(TIMEDELAY);
}

void stopMove() {
  leftWheel.stop();
  rightWheel.stop();
}

void forwardGrid() {
  for (int i = 0; i < delayGrid; i++) {
    forward();
  }
  stopMove();
}

void turnRight() {
  leftWheel.run(-motorSpeed);
  rightWheel.run(-motorSpeed);
  delay(TIMETURN);
  stopMove();
}

void turnLeft() {
  leftWheel.run(motorSpeed);
  rightWheel.run(motorSpeed);
  delay(TIMETURN);
  stopMove();
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

void reAdjust(float rightDist, float leftDist) {
  if (rightDist < SIDETHRESHOLD) reAdjustLeft();
  else reAdjustRight();
}

void reAdjustRight() {
  leftWheel.run(-motorSpeed - 50);
  rightWheel.run(motorSpeed);
  delay(TIMEDELAY);
}

void reAdjustLeft() {
  leftWheel.run(-motorSpeed);
  rightWheel.run(motorSpeed + 50);
  delay(TIMEDELAY);
}


// bool to check if too close to walls
int tooClose(float rightDist, float leftDist) {
  return rightDist < SIDETHRESHOLD || leftDist < SIDETHRESHOLD;
}


/* COLOUR FUNCTIONS */
// setBalance - calibrate between white and black
// setColours - set values of default colours into allColoursArray
// getColourValues - gets colour values of a given sample
// getColour - get colour from the default colours
// getAvgReading - gets direct reading from LDR

void setBalance() {
  //set white balance
  Serial.println("Put White Sample For Calibration ...");
  delay(5000);           //delay for five seconds for getting sample ready
  //digitalWrite(LED,LOW); //Check Indicator OFF during Calibration
  //scan the white sample.
  //go through one colour at a time, set the maximum reading for each colour -- red, green and blue to the white array
  for (int i = 0; i <= 2; i++) {
    int r = 0, g = 0, b = 0;
    switch (i) {
      case 0: r = 1; break;
      case 1: g = 1; break;
      case 2: b = 1; break;
    }
    led.setColor(r * MAXLED, g * MAXLED, b * MAXLED);
    led.show();
    delay(RGBWait);
    whiteArray[i] = getAvgReading(5);
    delay(RGBWait);
  }
  led.setColor(0, 0, 0);
  led.show();
  //done scanning white, time for the black sample.
  //set black balance
  Serial.println("Put Black Sample For Calibration ...");
  delay(5000);     //delay for five seconds for getting sample ready
  //go through one colour at a time, set the minimum reading for red, green and blue to the black array
  for (int i = 0; i <= 2; i++) {
    int r = 0, g = 0, b = 0;
    switch (i) {
      case 0: r = 1; break;
      case 1: g = 1; break;
      case 2: b = 1; break;
    }
    led.setColor(r * MAXLED, g * MAXLED, b * MAXLED);
    led.show();
    delay(RGBWait);
    blackArray[i] = getAvgReading(5);
    delay(RGBWait);
    //the differnce between the maximum and the minimum gives the range
    greyDiff[i] = whiteArray[i] - blackArray[i];
  }
  led.setColor(0, 0, 0);
  led.show();

  //delay another 5 seconds for getting ready colour objects
  Serial.println("Colour Sensor Is Ready.");
  delay(5000);
}

/*
void setColours(int colour) {
  switch (colour) {
    case 0: Serial.println("Put Red Sample For Calibration ..."); break;
    case 1: Serial.println("Put Green Sample For Calibration ..."); break;
    case 2: Serial.println("Put Yellow Sample For Calibration ..."); break;
    case 3: Serial.println("Put Purple Sample For Calibration ..."); break;
    case 4: Serial.println("Put Light Blue Sample For Calibration ..."); break;
  }
  delay(5000);
  for (int c = 0; c <= 2; c++) {
    getColourValues(c);
    allColourArray[colour][c] = colourArray[c];
    delay(RGBWait);
    Serial.println(int(colourArray[c])); //show the value for the current colour LED, which corresponds to either the R, G or B of the RGB code
  }
  led.setColor(0, 0, 0);
  led.show();
  delay(5000);
}
*/
int getColour() {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      getColourValues(j);
      delay(RGBWait);
      Serial.println(int(colourArray[j])); //show the value for the current colour LED, which corresponds to either the R, G or B of the RGB code
      
      /* TO CHANGE TO FINALCOLOURARRAY AFTER CALIBRATION */
      if (abs(allColourArray[i][j] - colourArray[j]) > COLOURTHRESHOLD)
        break;
      else if (j == 2)
        return i;
    }
  }
  return -1;
}

int getColourValues(int rgb) {
  Serial.print(colourStr[rgb]);
  int r = 0, g = 0, b = 0;
  switch (rgb) {
    case 0: r = 1; break;
    case 1: g = 1; break;
    case 2: b = 1; break;
  }
  led.setColor(r * MAXLED, g * MAXLED, b * MAXLED);
  led.show();//turn ON the LED, red, green or blue, one colour at a time.
  delay(RGBWait);
  //get the average of 5 consecutive readings for the current colour and return an average
  colourArray[rgb] = getAvgReading(5);
  //the average reading returned minus the lowest value divided by the maximum possible range, multiplied by 255 will give a value between 0-255, representing the value for the current reflectivity (i.e. the colour LDR is exposed to)
  colourArray[rgb] = (colourArray[rgb] - blackArray[rgb]) / (greyDiff[rgb]) * 255;
  led.setColor(0, 0, 0);
  led.show();
}



int getAvgReading(int times) {
  //find the average reading for the requested number of times of scanning LDR
  int reading;
  int total = 0;
  //take the reading as many times as requested and add them up
  for (int i = 0; i < times; i++) {
    reading = analogRead(LDR);
    total = reading + total;
    delay(LDRWait);
  }
  //calculate the average and return it
  return total / times;
}



// Function for waypoint checking using Line Sensor
// returns true if black strip
// returns false if no black strip

int checkWaypoint(int lineState) {
  // lineState values:
  // S1_IN_S2_IN
  // S1_IN_S2_OUT
  // S1_OUT_S2_IN
  // S1_OUT_S2_OUT
  // To decide which to use
  return frontDistance < FRNTTHRESHOLD && lineState == S1_IN_S2_IN;
}

/* WAYPOINT FUNCTIONS*/

void colourWaypoint(uint8_t colourRes) {
  switch (colourRes) {
    case 0: // Red
      turnLeft();
      break;
    case 1: // Green
      turnRight();
      break;
    case 2: // Yellow
      uTurn();
      break;
    case 3: // Purple
      doubleLeft();
      break;
    case 4: // LightBlue
      doubleRight();
      break;
  }
}

void soundWaypoint(uint8_t highStrength, uint8_t lowStrength) {
  // To do
}

void finish() {
  // To do
}
