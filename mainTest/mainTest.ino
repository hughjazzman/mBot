#include "MeMCore.h"
#include "Wire.h"
#include "Notes.h"

// Done:
// general movement - values calibrated (Wira)
// celebratory tune - tested, working (Shuyi)
// colour sensor -  values calibrated (Wira)
// IR side sensors (?) - calibrating (Walter)

// Half Done:


// Not done:
// sound sensors - (ZiHao) - ggwp



// determined by experiment:
// WAYPTDELAY - delay before decoding waypoint after detection
// TIMEDELAY - delay for how often position is checked
// TIMETURN - time needed for mBot to turn 90 deg
// TIMEGRID - time needed to travel 1 grid
// REDARR,GREARR,YELARR,PURARR,BLUARR - RGB Values of each colour stored in arrays
// COLOURTHRESHOLD - max deviation from calibrated colour values

// to be determined by experiment:
// SNDTHRESHOLD - voltage (V) value threshold for a sound
// FRNTTHRESHOLD - distance (cm) in front of mBot there is a wall to not crash into
// SIDETHRESHOLD - voltage (V) corresponding to closeness to side walls
// ADJTHRESHOLD - motorSpeed adjustment when detecting side walls
// SNDTHRESHOLD - when sound value is above a threshold, do sound waypoint
// SNDSAMPLE - no. of times sample the value obtained from sound filter 

#define MOTORSPEED 100
#define WAYPTDELAY 100
#define SNDTHRESHOLD 500
#define TIMEDELAY 20
#define TIMETURN 86000/MOTORSPEED
#define TIMEGRID 220000/MOTORSPEED
#define FRNTTHRESHOLD 5 // ultra to check
#define SIDETHRESHOLD 425 // IR to check
#define LEFTTHRESHOLD 425 // IR to check
#define RIGHTTHRESHOLD 325 // IR to check
#define ADJTHRESHOLD 5 // IR to check
#define SNDTHRESHOLD 3.5
#define SNDSAMPLE 10


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

#define COLOURTHRESHOLD 20 //max deviation from calibrated colour values

// Calibrated values for each colour used in colour waypoint test
// Values retrieved from colourcal.ino file after calibration
// Values placed in finalColVal array to be used in logic below
// Final values determined on
// WHIVAL and BLAVAL are white and black values from the light sensor
// using analogRead()
// To be close to the day itself
// Rest of colours should be fixed before then
#define WHIVAL 620 // value from LDR
#define BLAVAL 530 // value from LDR
#define REDVAL 45 //fixed
#define GREVAL 160 //fixed
#define YELVAL 230 //to check
#define PURVAL 260 //to check
#define BLUVAL 380 //to check
#define BLAVAL 0
#define NUMCOL 6



/*MCORE OBJECTS*/

// Motor setup (movement)
MeDCMotor leftWheel(M1);
MeDCMotor rightWheel(M2);
// Line Follower (Black Strip) setup
MeLineFollower lineFinder(PORT_2);
// Ultrasonic Sensor setup
MeUltrasonicSensor ultraSensor(PORT_1);
// Color Sensor setup
MeRGBLed led(7); // pin 7 is the RGB LED (WHY??)
// Speaker for celebratory tune
MeBuzzer buzzer;

// IR Sensor setup done from pins in setup()
// Sound Sensor setup done from pins in setup()

/* VARIABLES */

// Used in Motor
uint8_t motorSpeed = MOTORSPEED;
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
float colourArray = 0;
float whiteArray = WHIVAL;
float blackArray = BLAVAL;
float greyDiff = WHIVAL - BLAVAL;
float finalColVal[NUMCOL] = { BLAVAL, REDVAL, GREVAL, YELVAL, PURVAL, BLUVAL }; // black, red,green,yellow,purple,lightblue

char colourStr[3][5] = {"R = ", "G = ", "B = "};

//resulting colour at waypoints (calibration done before start)
int colourRes = 0;

/*ULTRASONIC SENSOR VARIABLES*/
// Used in Ultrasonic Sensor
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
//  moveTest();
//  lineTest();
//  ultraIRTest();
// colourTest();
//  soundTest();
  lineState = lineFinder.readSensors(); // Detection black strip below
  waypoint = checkWaypoint(lineState); // Presence of black strip
  /* Actual logic here */
  // If waypoint detected, decode it
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
  leftWheel.run(-motorSpeed);
  rightWheel.run(motorSpeed);
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

// Readjusting algorithms from wall detection
void reAdjust(float rightDist, float leftDist) {
  if (rightDist < SIDETHRESHOLD) reAdjustLeft();
  else reAdjustRight();
}

void reAdjustRight() {
  leftWheel.run(-motorSpeed - ADJTHRESHOLD);
  rightWheel.run(motorSpeed);
  delay(TIMEDELAY);
}

void reAdjustLeft() {
  leftWheel.run(-motorSpeed);
  rightWheel.run(motorSpeed + ADJTHRESHOLD);
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
// printColour - prints detected colour for troubleshooting


void printColour(int colourRes) {
  String s;
  switch (colourRes){
    case 0:
      s="black";
      break;
    case 1:
      s="red";
      break;
    case 2:
      s="green";
      break;
    case 3:
      s="yellow";
      break;
    case 4:
      s="purple";
      break;
    case 5:
      s="light blue";
      break;
    default:
      s="not found";
      break;
  }
  Serial.println(s);
}

void setBalance() {
	//set white balance
	Serial.println("Put White Sample For Calibration ...");
	delay(5000);           //delay for five seconds for getting sample ready
	//scan the white sample.
	//go through one colour at a time, set the maximum reading for each colour -- red, green and blue to the white array
	led.setColor(0, MAXLED, MAXLED);
	led.show();
	delay(RGBWait);
	whiteArray = getAvgReading(5);
	Serial.println(whiteArray);
	delay(RGBWait);
	//done scanning white, time for the black sample.
	//set black balance
	Serial.println("Put Black Sample For Calibration ...");
	delay(5000);     //delay for five seconds for getting sample ready
	led.setColor(0, MAXLED, MAXLED);
	led.show();
	delay(RGBWait);
	blackArray = getAvgReading(5);
	Serial.println(blackArray);
	delay(RGBWait);
	//the differnce between the maximum and the minimum gives the range
	greyDiff = whiteArray - blackArray;

	//delay another 5 seconds for getting ready colour objects
	Serial.println("Colour Sensor Is Ready.");
	delay(5000);
}

int getColour() {
  int curr, next;
  getColourValues();
  Serial.println(int(colourArray)); //show the value for the current colour LED
  // If no valid value return invalid
  if (colourArray < -30 || colourArray > 450)
    return -1;
  
  for (int i = 0; i < NUMCOL; i++) {
    // If reached range of value for last colour (blue), return 
    if (i == 6)
      return i;
    curr = finalColVal[i];
    next = finalColVal[i+1];
    
    if (colourArray > finalColVal[i] && colourArray < finalColVal[i + 1]) {
      if (colourArray <= (finalColVal[i] + finalColVal[i + 1]) / 2)
        return i;
      else
        return i + 1;
    }
  }
}

void getColourValues() {
	delay(RGBWait);
	//get the average of 5 consecutive readings for the current colour and return an average
	colourArray = getAvgReading(5);
	//the average reading returned minus the lowest value divided by the maximum possible range, multiplied by 255 will give a value between 0-255, representing the value for the current reflectivity (i.e. the colour LDR is exposed to)
	colourArray = (colourArray - blackArray) / (greyDiff) * 255;
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

int isSound() {
  for (int i=0; i<SNDSAMPLE; i++) {
    if (valueLow[i] > SNDTHRESHOLD) {
      return 1;
    }
    else if (valueHigh[i] > SNDTHRESHOLD) {
      return 2;
    }
  }
  return 0;
}


/* WAYPOINT CHECK*/
// Function for waypoint checking using Line Sensor
// returns true if black strip
// returns false if no black strip
int checkWaypoint(int lineState) {
  // lineState values (S1 and S2 are the 2 sensors - in or out the black line):
  
  // S1_IN_S2_IN
  // S1_IN_S2_OUT
  // S1_OUT_S2_IN
  // S1_OUT_S2_OUT
  // To decide which to use
  return lineState != S1_OUT_S2_OUT;
}

/* WAYPOINT FUNCTIONS*/
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

// Called when finish is detected, celebratory music played
// play mario theme song
// melody sheet by: Dipto Pratyaksa
// buzzer seems to be pin8
void celebratory_music(){
 int melody[] = {
    NOTE_E7, NOTE_E7, 0, NOTE_E7, 
    0, NOTE_C7, NOTE_E7, 0,
    NOTE_G7, 0, 0,  0,
    NOTE_G6, 0, 0, 0, 

    NOTE_C7, 0, 0, NOTE_G6, 
    0, 0, NOTE_E6, 0, 
    0, NOTE_A6, 0, NOTE_B6, 
    0, NOTE_AS6, NOTE_A6, 0, 

    NOTE_G6, NOTE_E7, NOTE_G7, 
    NOTE_A7, 0, NOTE_F7, NOTE_G7, 
    0, NOTE_E7, 0,NOTE_C7, 
    NOTE_D7, NOTE_B6, 0, 0,

    NOTE_C7, 0, 0, NOTE_G6, 
    0, 0, NOTE_E6, 0, 
    0, NOTE_A6, 0, NOTE_B6, 
    0, NOTE_AS6, NOTE_A6, 0, 

    NOTE_G6, NOTE_E7, NOTE_G7, 
    NOTE_A7, 0, NOTE_F7, NOTE_G7, 
    0, NOTE_E7, 0,NOTE_C7, 
    NOTE_D7, NOTE_B6, 0, 0
 };
 
 int noteDurations[] = {
    12, 12, 12, 12, 
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12, 

    12, 12, 12, 12,
    12, 12, 12, 12, 
    12, 12, 12, 12, 
    12, 12, 12, 12, 

    9, 9, 9,
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,
  
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,

    9, 9, 9,
    12, 12, 12, 12,
    12, 12, 12, 12,
    12, 12, 12, 12,
 };

 // loop until all values in the melody array have been used
 // ie. when i reaches size of melody array
 int size = sizeof(melody)/sizeof(int);
 for (int i = 0; i < size; i++) {
    
    // to calculate the note duration, take one second
    // divided by the note type.
    // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc. (Assuming 1 beat per sec)
    int noteDuration = 1000/noteDurations[i];
    buzzer.tone(8, melody[i], noteDuration);
    
    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    
    // stop the tone playing:
    buzzer.noTone(8);
 }
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
  if (lineState != S1_OUT_S2_OUT){
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
