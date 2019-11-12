#include <Wire.h>
#include <MeMCore.h>
#include <MeRGBLed.h>
#include "Notes.h"
using ll = long long;

/********** Settings **********/
#define LEFTIR_PIN      A1
#define RIGHTIR_PIN     A2
#define SNDLOW_PIN      A3                      // 100-300
#define SNDHI_PIN       A0                      // >3000
#define LDR_PIN         A6
#define LED_PIN         7
#define MUSIC_PIN       8
MeDCMotor               leftWheel(M1);          // -255 to 255
MeDCMotor               rightWheel(M2);         // 255 to -255, ie reversed
MeLineFollower          lineFinder(PORT_2);     // PORT_3
// MeColorSensor           colorSensor(PORT_1);
MeUltrasonicSensor      ultraSensor(PORT_1);    // PORT_7
MeSoundSensor           highSound(PORT_6);
MeSoundSensor           lowSound(PORT_5);
MeRGBLed                led(LED_PIN);
MeBuzzer                buzzer;

// General
#define WAYPTDELAY      100                     // delay b4 decoding challenge
#define TIMEDELAY       20                      // delay b4 recheck position

// Movement
#define MOTORSPEED      100
#define TIMETURN        (75000/MOTORSPEED)      // time for 90deg turn
#define TIMEGRID        (220000/MOTORSPEED)     // time to travel 1 grid
#define DELAYGRID       (TIMEGRID / TIMEDELAY)
#define K_ERR           0.5
#define K_DIST          (255/2)                 // max correction to mvmt
#define LEFT_BIAS       0                       // 128
#define FRONT_BIAS      5                       // todo

// Sound
#define K_SNDLOW        40
#define K_SNDHI         10

// Color
// retrieved from colourcal.ino file after calibration
#define MIN_DIST        10000
#define WHI_VAL         {375, 335, 380}         // from LDR b4 normalisation
#define BLA_VAL         {315, 265, 305}
#define GRE_VAL         {60,70,75}
#define RED_ARR         {185,35,35}             // normalised rgb vals
#define GRE_ARR         {45, 100, 60}
#define YEL_ARR         {255, 175, 100}
#define PUR_ARR         {115, 110, 175}
#define BLU_ARR         {140, 200, 230}
#define BLA_ARR         {0,0,0}
#define NUMCOL          6                       // black, red, green, yellow, purple, blue

// Calibration
#define CALLIBRATE_SEC  2                       // delay b4 calibration
#define CALIBRATE_NO    10                      // no of measurements
#define IR_WAIT         100                     // delay btw measurements. IMPT!
#define RGB_WAIT        200
#define LDR_WAIT        10
#define MIC_WAIT        100
#define LED_MAX         255
#define IR_MAX          1023



/********** Global Variables **********/
bool busy = true;

int IR_VALUES[2][2] = {0}; // left-right, minmax
ll error = 0;

int whiteArray[] = WHI_VAL;
int blackArray[] = BLA_VAL;
int greyDiff[] = GRE_VAL;
static int allColourArray[6][3] = {BLA_ARR, RED_ARR,GRE_ARR,YEL_ARR,PUR_ARR,BLU_ARR};
  


/********** Main Program **********/
void setup() {
  pinMode(LEFTIR_PIN, INPUT);
  pinMode(RIGHTIR_PIN, INPUT);
  pinMode(SNDLOW_PIN, INPUT);
  pinMode(SNDHI_PIN, INPUT);
  Serial.begin(9600);

  calibrateWB();
  calibrateIR();
  // colorSensor.SensorInit();
  busy = false;
}

void stopMove(int i);
void loop() {
  if (busy) return;

  // double frontDistance = ultraSensor.distanceCm();
  if (lineFinder.readSensors() != S1_IN_S2_IN) { // both sensors not in black line
    moveForward(); // todo: front
    return;
  }
  
  // Waypoint detected!
  busy = true;
  stopMove(0);
  delay(WAYPTDELAY);

  // Color Challenge
  int colourRes;
  do {
    colourRes = getColour();
  } while (colourRes == -1);
  if (colourRes > 0) { // is color challenge (not black)
    colorWaypoint(colourRes);
    busy = false;
    return;
  }

  // Sound Challenge
  int soundRes = getSound();
  if (soundRes > 0) {
    soundWaypoint(soundRes);
    busy = false;
    return;
  }

  // Finished!
  finishWaypoint();
}



/********** Movement **********/
void stopMove(int i = 10) {
  rightWheel.stop();
  leftWheel.stop();
  if (i) delay(TIMEDELAY * i);
}

void moveForward() {
  for (int i = 0; i < DELAYGRID; ++i) {
    int dx = getDist();
    
    // Normalise to MOTORSPEED
    int maxx = MOTORSPEED + (dx >= 0 ? dx : -dx);
    leftWheel.run((ll)(-MOTORSPEED + dx) * MOTORSPEED / maxx);
    rightWheel.run((ll)(MOTORSPEED + dx) * MOTORSPEED / maxx);
    
    delay(TIMEDELAY);
  }
  stopMove(0);

  // if (checkFront(frontDistance)){
  //   if (rightDist > leftDist){
  //     turnRight();
  //   }
  //   else {
  //     turnLeft();
  //   }
  // }
  // // Readjust if too close to right or left wall
  // else if (tooClose(rightDist, leftDist)) {
  //   reAdjust(rightDist, leftDist);
  // } 
  // // Default movement: forward 
  // else {
  //   forward();
  // }
}

void forward() {
  leftWheel.run(-MOTORSPEED);
  rightWheel.run(MOTORSPEED);
  delay(TIMEDELAY);
}

void forwardGrid() {
  for (int i = 0; i < DELAYGRID; ++i) { forward(); }
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



/********** Sensors **********/
// IR Sensor: Proximity
int getDist() {
  // Position
  // If left > right, too close to right, turn left
  int left = analogRead(LEFTIR_PIN), right = analogRead(RIGHTIR_PIN);
  left = norm(left, IR_VALUES[0]) - LEFT_BIAS;
  right = norm(right, IR_VALUES[1]);

  // Only care if too close to either side
  if (left > 877) left = 1; // so we know it's not 0
  if (right > 877) right = 1;
  Serial.print("LEFT: "); Serial.print(left);
  Serial.print("\tRIGHT: "); Serial.println(right);

  // TODO: Check front
  // Ultrasonic

  ll curr = (ll)(left - right) * K_DIST / 1023;
  // return curr;
  curr -= error;
  error += curr;
  return curr + error * K_ERR;
}

// Sound Sensor: Return first hz
int getSound() {
  for (int i = 0; i < CALIBRATE_NO; ++i) {
    int low = analogRead(SNDLOW_PIN);
    int hi = analogRead(SNDHI_PIN);
    if (low > K_SNDLOW) return 1;
    else if (hi > K_SNDHI) return 2;
    delay(MIC_WAIT);
  }
  return 0;
}

// Color Sensor: Return nearest colour
ll square(ll x) { return x * x; }
int getColour() { // returns index of best color
  float colourArray[3] = {0};
  for (int i = 0; i < 3; ++i) {
    led.setColor( // one-hot encoding
      ((1<<i)   &1) * LED_MAX,
      ((1<<i>>1)&1) * LED_MAX,
      ((1<<i>>2)&1) * LED_MAX
    ); led.show();
    delay(RGB_WAIT);

    for (int i = 0; i < CALIBRATE_NO; ++i) {
      colourArray[i] += analogRead(LDR_PIN);
      delay(LDR_WAIT);
    }
    colourArray[i] /= CALIBRATE_NO;
    colourArray[i] = (colourArray[i] - blackArray[i]) * 255 / greyDiff[i];
  }
  led.setColor(0,0,0); led.show();

  int idx = -1, min_dist = MIN_DIST;
  for (int i = 0; i < 6; ++i) {
    ll curr_dist = 0;
    for (int j = 0; j < 3; ++j) {
      curr_dist += square(allColourArray[i][j] - colourArray[j]);
    }
    if (min_dist > curr_dist && curr_dist > 0) {
      idx = i; min_dist = curr_dist;
    }
  }

  return idx;
}

// void getColours(uint16_t colorvalues[]) {
//   colorvalues[0] = colorSensor.Returnresult();
//   colorvalues[1] = colorSensor.ReturnRedData();
//   colorvalues[2] = colorSensor.ReturnGreenData();
//   colorvalues[3] = colorSensor.ReturnBlueData();
//   colorvalues[4] = colorSensor.ReturnColorData();
// }

// void colorPrint() {
//   uint16_t colorvalues[5];
//   getColours(colorvalues);
//   long colorcode = colorSensor.ReturnColorCode();//RGB24code
//   uint8_t grayscale = colorSensor.ReturnGrayscale();

//   Serial.print("R:"); Serial.print(colorvalues[1]); Serial.print("\t");
//   Serial.print("G:"); Serial.print(colorvalues[2]); Serial.print("\t");
//   Serial.print("B:"); Serial.print(colorvalues[3]); Serial.print("\t");
//   Serial.print("C:"); Serial.print(colorvalues[4]); Serial.print("\t");
//   Serial.print("color:");
//   switch (colorvalues[0])
//   {
//     case BLACK: Serial.print("BLACK"); break;
//     case BLUE: Serial.print("BLUE"); break;
//     case YELLOW: Serial.print("YELLOW"); break;
//     case GREEN: Serial.print("GREEN"); break;
//     case RED: Serial.print("RED"); break;
//     case WHITE: Serial.print("WHITE"); break;
//   }
//   Serial.print("\t");
//   Serial.print("code:"); Serial.print(colorcode, HEX); Serial.print("\t");
//   Serial.print("grayscale:"); Serial.println(grayscale);
// }



/********** Waypoints **********/
void colorWaypoint(int colourRes) {
  // red : left
  // green : right
  // yellow : 180deg within grid
  // purple : 2 left
  // light blue : 2 right
  switch (colourRes) {
    case 1: turnLeft(); break;
    case 2: turnRight(); break;
    case 3: uTurn(); break;
    case 4: doubleLeft(); break;
    case 5: doubleRight(); break;
  }
}

void soundWaypoint(int soundRes) {
  // 1  low (100-300) : left
  // 2  right (>3000) : right
  switch (soundRes) {
    case 1: turnLeft(); break;
    case 2: turnRight(); break;
  }
}

void finishWaypoint() {
  // stop moving, play sound
  for (int i = 0; i < sizeof(music_key) / sizeof(int); ++i) {
    // quarter note = 1000 / 4, eighth note = 1000/8, etc. (Assuming 1 beat per sec)
    int duration = 1000 / music_duration[i];
    buzzer.tone(MUSIC_PIN, music_key[i], duration);

    // to distinguish notes
    delay(duration * 1.30);
    buzzer.noTone(MUSIC_PIN);
  }
}



/********** Calibration **********/
int norm(const int val, const int *low_mult) {
  int tmp = 1023LL * (val - *low_mult) / *(low_mult+1); // int will overflow
  if (tmp < 0) tmp = 0;
  else if (tmp > 1023) tmp = 1023;
  return tmp;
}

void calibrateIR() {
  Serial.println("===== CALIBRATING IR SENSORS (SIDE) =====");

  // Min values
  Serial.print("COVER SENSORS. Calibrating MIN in ");
  for (int i = CALLIBRATE_SEC; i > 0; --i) {
    Serial.print(i); Serial.print(".. "); delay(1000);
  }
  for (int i = 0; i < CALIBRATE_NO; ++i) {
    IR_VALUES[0][0] += analogRead(LEFTIR_PIN);
    IR_VALUES[1][0] += analogRead(RIGHTIR_PIN);
    delay(IR_WAIT);
  }
  IR_VALUES[0][0] /= CALIBRATE_NO;
  IR_VALUES[1][0] /= CALIBRATE_NO;
  Serial.println("done.");

  // Max values
  Serial.print("UNCOVER SENSORS. Calibrating MAX in ");
  for (int i = CALLIBRATE_SEC; i > 0; --i) {
    Serial.print(i); Serial.print(".. "); delay(1000);
  }
  for (int i = 0; i < CALIBRATE_NO; ++i) {
    IR_VALUES[0][1] += analogRead(LEFTIR_PIN);
    IR_VALUES[1][1] += analogRead(RIGHTIR_PIN);
    delay(IR_WAIT);
  }
  IR_VALUES[0][1] /= CALIBRATE_NO;
  IR_VALUES[1][1] /= CALIBRATE_NO;
  Serial.println("done.\n");

  // Save range
  // Serial.println(IR_VALUES[0][0]);
  // Serial.println(IR_VALUES[0][1]);
  // Serial.println(IR_VALUES[1][0]);
  // Serial.println(IR_VALUES[1][1]);
  IR_VALUES[0][1] -= IR_VALUES[0][0]; // left range
  IR_VALUES[1][1] -= IR_VALUES[1][0]; // right range
}

void calibrateWB() {
  Serial.println("===== CALIBRATING COLOR SENSORS (TOP) =====");

  // Max values
  Serial.print("Put WHITE sample. Calibrating MAX in ");
  for (int i = CALLIBRATE_SEC; i > 0; --i) {
    Serial.print(i); Serial.print(".. "); delay(1000);
  }
  for (int i = 0; i < 3; ++i) {
    led.setColor( // one-hot encoding
      ((1<<i)   &1) * LED_MAX,
      ((1<<i>>1)&1) * LED_MAX,
      ((1<<i>>2)&1) * LED_MAX
    ); led.show();
    delay(RGB_WAIT);

    whiteArray[i] = 0;
    for (int i = 0; i < CALIBRATE_NO; ++i) {
      whiteArray[i] += analogRead(LDR_PIN);
      delay(LDR_WAIT);
    }
    whiteArray[i] /= CALIBRATE_NO;
  }
  led.setColor(0,0,0); led.show();
  Serial.println("done.");


  // Min values
  Serial.print("Put BLACK sample. Calibrating MIN in ");
  for (int i = CALLIBRATE_SEC; i > 0; --i) {
    Serial.print(i); Serial.print(".. "); delay(1000);
  }
  for (int i = 0; i < 3; ++i) {
    led.setColor( // one-hot encoding
      ((1<<i)   &1) * LED_MAX,
      ((1<<i>>1)&1) * LED_MAX,
      ((1<<i>>2)&1) * LED_MAX
    ); led.show();
    delay(RGB_WAIT);
    
    blackArray[i] = 0;
    for (int i = 0; i < CALIBRATE_NO; ++i) {
      blackArray[i] += analogRead(LDR_PIN);
      delay(LDR_WAIT);
    }
    blackArray[i] /= CALIBRATE_NO;
    greyDiff[i] = whiteArray[i] - blackArray[i];
  }
  led.setColor(0,0,0); led.show();
  Serial.println("done.");
}
