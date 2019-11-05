#include "MeMCore.h"
#include "Wire.h"

// Done:
// general movement - values calibrated (Wira)
// celebratory tune - not tested (Shuyi)

// Half Done:
// colour sensor (?) - to be calibrated (Wira)

// Not done:
// IR side sensors (?) -not tested (Walter)
// sound sensors - (ZiHao)



// determined by experiment:
// WAYPTDELAY - delay before decoding waypoint after detection
// TIMEDELAY - delay for how often position is checked, should be a factor of TIMEGRID
// TIMETURN - time needed for mBot to turn 90 deg
// TIMEGRID - time needed to travel 1 grid

// to be determined by experiment:
// REDARR,GREARR,YELARR,PURARR,BLUARR - RGB Values of each colour stored in arrays
// SNDTHRESHOLD - voltage (V) value threshold for a sound
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
#define REDARR {0,0,0}
#define GREARR {0,0,0}
#define YELARR {0,0,0}
#define PURARR {0,0,0}
#define BLUARR {0,0,0}

/* NOTES FOR CELEBRATORY TUNE */
//notes
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

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
// Sound Sensor setup done from pins in setup()
// Speaker for celebratory tune
MeBuzzer buzzer;

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
  	  highStrength = analogRead(SNDHI); // Get high f sound strength
  	  lowStrength = analogRead(SNDLOW); // Get low f sound strength

  	  // If color waypoint
  	  if (colourRes >= 0)
  		  colourWaypoint(colourRes);
  	  // If sound waypoint
  	  //else if (highStrength > SNDTHRESHOLD || lowStrength > SNDTHRESHOLD)
  		  //soundWaypoint(highStrength, lowStrength);
  	  // If finished
  	  else
  		  celebratory_music();
    }

    // If no waypoint
    
    if (tooClose(rightDist, leftDist)) {
  	  reAdjust(rightDist, leftDist);
    }
    else if (checkFront(frontDistance)){
      if (rightDist > leftDist){
        turnRight;
      }
      else{
        turnLeft;
      }
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

// Checks if too close to front wall 
int checkFront(int frontDistance){
    return frontDistance < FRNTTHRESHOLD;
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
  return lineState == S1_IN_S2_IN;
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
  if (highStrength > SNDTHRESHOLD)
    turnRight();
  else if (lowStrength > SNDTHRESHOLD)
    turnLeft();
}

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
