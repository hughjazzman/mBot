#include "MeMCore.h"
#include "Wire.h"
#include "Notes.h"

// general movement - values calibrated (Wira)
// celebratory tune - tested, working (Shuyi)
// colour sensor -  values calibrated (Wira)
// IR side sensors (?) - calibrating (Walter)
// sound sensors - (ZiHao)


/*PIN NUMBERS*/
// Infrared side sensor pins
#define IRL A1 //left
#define IRR A2 //right

// Sound sensor pins
#define SNDLOW  A3 //low
#define SNDHI   A0 //high

//LDR sensor pin at A6
#define LDR A6   


/* MOVEMENT CONSTANTS */
// Can be changed:
// MOTORSPEED - decides MOTORSPEED

// determined by experiment:
// WAYPTDELAY - delay before decoding waypoint after detection
// TIMEDELAY - delay for how often position is checked
// TIMETURN - time needed for mBot to turn 90 deg
// TIMEGRID - time needed to travel 1 grid

#define MOTORSPEED  100
#define WAYPTDELAY  100
#define TIMEDELAY   20
#define TIMETURN    75000/MOTORSPEED
#define TIMEGRID    220000/MOTORSPEED

// Movement adjustment constants
// To be changed by Walter (just use your own code)
// FRNTTHRESHOLD - distance (cm) in front of mBot there is a wall to not crash into
// SIDETHRESHOLD - voltage (V) corresponding to closeness to side walls
// ADJTHRESHOLD - MOTORSPEED adjustment when detecting side walls
#define FRNTTHRESHOLD 5
#define SIDETHRESHOLD 0.5
#define ADJTHRESHOLD  50

/* SOUND CONSTANTS */
// LOWTHRESHOLD,HIGHTHRESHOLD - when sound value is above a threshold, do sound waypoint
// SNDSAMPLE - no. of times sample the value obtained from sound filter 

#define LOWTHRESHOLD  40
#define HIGHTHRESHOLD 10
#define SNDSAMPLE     10

/* COLOR CONSTANTS*/
// Adapted from CG1111 Lab
// RGBWait - Define time delay before the next RGB colour turns ON to allow LDR to stabilize
// LDRWait - Define time delay before taking another LDR reading
// MAXLED - max value of LED RGB values

#define RGBWait 200 //in milliseconds 
#define LDRWait 10 //in milliseconds 
#define MAXLED  255



// Calibrated values for each colour used in colour waypoint test
// Values retrieved from colourcal.ino file after calibration
// Values placed in allColourArray array to be used in logic below
// values determined on 08 Nov 19
// WHIVAL, BLAVAL,GREVAL - White and Black values from the LDR (prenormalisation)
// REDARR,GREARR,YELARR,PURARR,BLUARR - RGB Values of each colour stored in arrays (normalised)
#define WHIVAL {375, 335, 380}
#define BLAVAL {315, 265, 305}
#define GREVAL {60,70,75}
#define REDARR {185,35,35}
#define GREARR {45, 100, 60}
#define YELARR {255, 175, 100}
#define PURARR {115, 110, 175}
#define BLUARR {140, 200, 230}
#define BLAARR {0,0,0}
#define NUMCOL 6 // number of colours in allColourArray - black, red, green, yellow, purple, blue

// IR Sensor setup done from pins in setup()
// Sound Sensor setup done from pins in setup()

/* VARIABLES */

// Used in movement
// no. of TIMEDELAY cycles to travel 1 grid
int delayGrid = TIMEGRID / TIMEDELAY;

// Walter to change 
// Used in IR Sensor
// value comes from IR sensor in voltage
float rightDist = 0;
float leftDist = 0;

// Used in Line Follower
int lineState; // to detect black strip

// Used for Colour Detection
//placeholders for colour detected
int red = 0;
int green = 0;
int blue = 0;

//floats to hold colour arrays
//allColourArray holds precalibrated RGB values
float colourArray[] = {0, 0, 0};
float whiteArray[] = WHIVAL;
float blackArray[] = BLAVAL;
float greyDiff[] = GREVAL;
int allColourArray[6][3] = {BLAARR, REDARR,GREARR,YELARR,PURARR,BLUARR};

char colourStr[3][5] = {"R = ", "G = ", "B = "};

//resulting colour at waypoints (calibration done before start)
int colourRes = 0;

// Used in Ultrasonic Sensor (Wakter)
double frontDistance;

// Used in Sound Sensor
double valueLow[SNDSAMPLE], valueHigh[SNDSAMPLE];

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
  led.setColor(0, MAXLED,  MAXLED);
  led.show();
  //colour calibration first (black and white)
  //setBalance();
}

void loop() {
  //moveTest();
//  lineTest();
//  ultraIRTest();
 //colourTest();
//  soundTest();
  //celebratory_music();
  lineState = lineFinder.readSensors(); // Detection black strip below
  waypoint = checkWaypoint(lineState); // Presence of black strip
  /* Actual logic here */
  // If waypoint detected, decode it
  /*
  if (waypoint) {
    stopMove();
    delay(WAYPTDELAY); // Delay before start
    colourRes = getColour(); // Get preset colour result
    soundVal(); // get sound values

    // Waypoint decoding
    // will only finish this loop if a colour is detected
    while (colourRes == -1) {
      colourRes = getColour(); // Get preset colour result
      printColour(colourRes);
    }
    // If color waypoint
    if (colourRes > 0)
      colourWaypoint(colourRes);
    // If sound waypoint
    else if (isSound())
      soundWaypoint(isSound());
    // If finished
    else
      celebratory_music();
  }
  else
    forward();
    */
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
// reAdjust
// reAdjustRight
// reAdjustLeft
// tooClose
// checkFront

void forward() {
  leftWheel.run(-MOTORSPEED);
  rightWheel.run(MOTORSPEED);
  delay(TIMEDELAY);
}

void stopMove() {
  rightWheel.stop();
  leftWheel.stop();
  delay(TIMEDELAY*10);
}

void forwardGrid() {
  for (int i = 0; i < delayGrid; i++) {
    forward();
  }
  stopMove();
}

void turnRight() {
  leftWheel.run(-MOTORSPEED);
  rightWheel.run(-MOTORSPEED);
  delay(TIMETURN);
  stopMove();
}

void turnLeft() {
  leftWheel.run(MOTORSPEED);
  rightWheel.run(MOTORSPEED);
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

// Walter
// Readjusting algorithms from wall detection
void reAdjust(float rightDist, float leftDist) {
  if (rightDist < SIDETHRESHOLD) reAdjustLeft();
  else reAdjustRight();
}

void reAdjustRight() {
  leftWheel.run(-MOTORSPEED - ADJTHRESHOLD);
  rightWheel.run(MOTORSPEED);
  delay(TIMEDELAY);
}

void reAdjustLeft() {
  leftWheel.run(-MOTORSPEED);
  rightWheel.run(MOTORSPEED + ADJTHRESHOLD);
  delay(TIMEDELAY);
}

// bool to check if too close to walls
int tooClose(float rightDist, float leftDist) {
  return rightDist < SIDETHRESHOLD || leftDist < SIDETHRESHOLD;
}

// Checks if too close to front wall 
int checkFront(int frontDistance){
    return frontDistance < FRNTTHRESHOLD;
}


/* COLOUR FUNCTIONS */
// setBalance - calibrate between white and black
// getColour - get colour from the default colours
// getColourValues - gets colour values of a given sample
// getAvgReading - gets direct reading from LDR

void setBalance() {
  //set white balance
  Serial.println("Put White Sample For Calibration ...");
  delay(5000);           //delay for five seconds for getting sample ready
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

int getColour() {
  int idx=-1,min_dist=10000;
  long long curr_dist;

  // Gets RGB values for current colour
  for (int x=0; x<3; x++) {
    getColourValues(x);
    Serial.println(colourArray[x]);
  }

  // Loop finds colour with smallest sum of square values difference
  for (int i = 0; i < 6; i++) {
    curr_dist = 0;
    // Takes the sum of square values of difference between current colour detected
    // and reference values saved in allColourArray
    for (int j = 0; j < 3; j++) {
      curr_dist += (allColourArray[i][j]-colourArray[j])*(allColourArray[i][j]-colourArray[j]);
    }
    if (curr_dist < min_dist && curr_dist > 0) {
      // Sets colour index to be returned here
      idx = i;
      min_dist = curr_dist;
    }
  }
  return idx;
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

/* SOUND PEAK DETECTOR */
// soundVal - Function to check for sound, SNDSAMPLE checks to make sure correct sound frequency detected
// isSound - tells which sound is being played, if any

void soundVal() {
  for (int i = 0; i < SNDSAMPLE; i++) {
    valueLow[i] = analogRead(SNDLOW)/204.6;
    delay(100);
    valueHigh[i] = analogRead(SNDHI)/204.6;
    delay(100);
  }
}

// return value 1 - low frequency sound
// return value 2 - high frequency sound
int isSound() {
  for (int i=0; i<SNDSAMPLE; i++) {
    if (valueLow[i] > LOWTHRESHOLD) {
      return 1;
    }
    else if (valueHigh[i] > HIGHTHRESHOLD) {
      return 2;
    }
  }
  return 0;
}


/* WAYPOINT CHECK & FUNCTIONS*/
// Function for waypoint checking using Line Sensor
int checkWaypoint(int lineState) {
  // lineState values (S1 and S2 are the 2 sensors - in or out the black line):
  // S1_IN_S2_IN
  // S1_IN_S2_OUT
  // S1_OUT_S2_IN
  // S1_OUT_S2_OUT
  // whenever 1 sensor detected this returns true
  return lineState != S1_OUT_S2_OUT;
}

// Detects colour and moves appropriately
void colourWaypoint(uint8_t colourRes) {
  switch (colourRes) {
    case 0: // Black
      break;
    case 1: // Red
      turnLeft();
      break;
    case 2: // Green
      turnRight();
      break;
    case 3: // Yellow
      uTurn();
      break;
    case 4: // Purple
      doubleLeft();
      break;
    case 5: // LightBlue
      doubleRight();
      break;
  }
}

// Detects sound and moves appropriately
void soundWaypoint(int isSound) {
  if (isSound == 1)
    turnLeft();
  else if (isSound == 2)
    turnRight();
}



// Movement Tests
void moveTest() {
  forwardGrid();
  turnRight();
  turnLeft();
  uTurn();
  forwardGrid();
  doubleRight();
  forwardGrid();
  doubleLeft();
  forwardGrid();
  turnLeft();
  forwardGrid();
  forwardGrid();
  turnLeft();
}

// LineFinder Tests
void lineTest() {
  lineState = lineFinder.readSensors(); // Detection black strip below
  if (checkWaypoint(lineState)){
    stopMove();
    Serial.println("stop");
  }
  else {
    forward();
    Serial.println("forward");
  }
}

// Sensor movement test
void ultraIRTest() {
  frontDistance = ultraSensor.distanceCm(); // Distance to wall in front
  rightDist = analogRead(IRR);
  leftDist = analogRead(IRL);
  Serial.print("Right:");
  Serial.println(rightDist);
  Serial.print("Left:");
  Serial.println(leftDist);
  /*
  if (checkFront(frontDistance)){
    if (rightDist > leftDist){
      turnRight();
    }
    else {
      turnLeft();
    }
  }
  else if (tooClose(rightDist, leftDist)) {
    reAdjust(rightDist, leftDist);
  } 
  // Default movement: forward 
  else {
    forward();
  }
  */
}

// Colour decoding Test
void colourTest() {
  led.setColor(0,MAXLED,MAXLED);
  led.show();
  forward();
  lineState = lineFinder.readSensors(); // Detection black strip below
  if (lineState != S1_OUT_S2_OUT) {
    stopMove();
    colourRes = getColour();
    colourWaypoint(colourRes);
    delay(TIMEGRID); 
  }
}

// Sound decoding Test
void soundTest() {
  soundVal();
  soundWaypoint(isSound());
}
