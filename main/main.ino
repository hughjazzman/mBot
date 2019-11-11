#include <Wire.h>

#include "MeMCore.h"
#include "Wire.h"
#include <MeRGBLed.h>
using ll = long long;

/********** Settings **********/
#define LEFTIR_PIN      A1
#define RIGHTIR_PIN     A2
#define BAND1_PIN       A3      // 100-300
#define BAND2_PIN       A0      // >3000
MeDCMotor               leftWheel(M1);        // -255 to 255
MeDCMotor               rightWheel(M2);       // 255 to -255, ie reversed
MeLineFollower          lineFinder(PORT_3);
MeColorSensor           colorSensor(PORT_1);
MeUltrasonicSensor      ultraSensor(PORT_7);
MeSoundSensor           highSound(PORT_6);
MeSoundSensor           lowSound(PORT_5);

// General
#define WAYPTDELAY      500     // delay b4 decoding challenge
#define TIMEDELAY       500     // delay b4 recheck position

// Movement
#define LEFT_BIAS       0 //128
#define MOTORSPEED      100
#define TIMETURN        1100    // time for 90deg turn
#define TIMEGRID        2000    // time to travel 1 grid
#define K_ERR           0.5
#define K_DIST          (255/2) // max correction to mvmt
#define DELAYGRID       (TIMEGRID / TIMEDELAY)

// Sound
#define SNDTHRESHOLD    500     // sound threshold

// Calibration
#define CALLIBRATE_SEC  3       // delay b4 calibration
#define CALIBRATE_NO    10       // no of measurements
#define CALIBRATE_DELAY 100     // delay btw measurements. IMPT!



/********** Global Variables **********/
bool busy = true;
int IR_VALUES[2][2] = {0}; // left-right, minmax
ll error = 0;
  


/********** Main Program **********/
void setup() {
  Serial.begin(9600);
  calibrateIR();
  colorSensor.SensorInit();
  busy = false;
}

void loop() {
  if (busy) return;

  // double frontDistance = ultraSensor.distanceCm();

  if (lineFinder.readSensors() != S1_IN_S2_IN) { // both sensors not in black line
    // Detect Left-Right Proximity
    moveForward();
    return;
  }
  
  // Waypoint detected!  
  delay(WAYPTDELAY);

  // Color Challenge
  uint16_t colorvalues[5];
  getColours(colorvalues);
  if (colorvalues[0] != BLACK) { // is color challenge (not black)
    // colorWaypoint(colorvalues[0]);
    return;
  }

  // Sound Challenge
  uint8_t highStrength = highSound.strength();
  uint8_t lowStrength = lowSound.strength();
  if (highStrength > SNDTHRESHOLD || lowStrength > SNDTHRESHOLD) {
    // soundWaypoint(highStrength, lowStrength);
    return;
  }

  // finishWaypoint();
}

// DC Motor: Movement
void moveForward() {
  for (int i = 0; i < DELAYGRID; ++i) {
    int dx = IR();
    int maxx = MOTORSPEED + (dx >= 0 ? dx : -dx);
    leftWheel.run((ll)(-MOTORSPEED + dx) * MOTORSPEED / maxx);
    rightWheel.run((ll)(MOTORSPEED + dx) * MOTORSPEED / maxx);
    delay(TIMEDELAY);
    leftWheel.stop();
    rightWheel.stop();
  }
}

// IR Sensor: Proximity
int IR() {
  // Position
  // If left > right, going towards right, turn left
  int left = analogRead(LEFTIR_PIN), right = analogRead(RIGHTIR_PIN);
  left = norm(left, IR_VALUES[0]) - LEFT_BIAS;
  right = norm(right, IR_VALUES[1]);
  if (left > 877) left = 1; // so we know it's not 0
  if (right > 877) right = 1;

  Serial.print("LEFT: "); Serial.print(left);
  Serial.print("\tRIGHT: "); Serial.println(right);

  ll curr = (ll)(left - right) * K_DIST / 1023;
//  return curr;
  
  curr -= error;
  error += curr;
  return curr + error * K_ERR;
}

// Color Sensor: Challenge
void getColours(uint16_t colorvalues[]) {
  colorvalues[0] = colorSensor.Returnresult();
  colorvalues[1] = colorSensor.ReturnRedData();
  colorvalues[2] = colorSensor.ReturnGreenData();
  colorvalues[3] = colorSensor.ReturnBlueData();
  colorvalues[4] = colorSensor.ReturnColorData();
}

void colorPrint() {
  uint16_t colorvalues[5];
  getColours(colorvalues);
  long colorcode = colorSensor.ReturnColorCode();//RGB24code
  uint8_t grayscale = colorSensor.ReturnGrayscale();

  Serial.print("R:"); Serial.print(colorvalues[1]); Serial.print("\t");
  Serial.print("G:"); Serial.print(colorvalues[2]); Serial.print("\t");
  Serial.print("B:"); Serial.print(colorvalues[3]); Serial.print("\t");
  Serial.print("C:"); Serial.print(colorvalues[4]); Serial.print("\t");
  Serial.print("color:");
  switch (colorvalues[0])
  {
    case BLACK: Serial.print("BLACK"); break;
    case BLUE: Serial.print("BLUE"); break;
    case YELLOW: Serial.print("YELLOW"); break;
    case GREEN: Serial.print("GREEN"); break;
    case RED: Serial.print("RED"); break;
    case WHITE: Serial.print("WHITE"); break;
  }
  Serial.print("\t");
  Serial.print("code:"); Serial.print(colorcode, HEX); Serial.print("\t");
  Serial.print("grayscale:"); Serial.println(grayscale);
}



/********** Waypoints **********/
void colorWaypoint() {
  // red : left
  // green : right
  // yellow : 180deg within grid
  // purple : 2 left
  // light blue : 2 right
  return;
}

void soundWaypoint() {
  // black
  // low (100-300) : left
  // right (>3000) : right
  return;
}

void finishWaypoint() {
  // black, no sound
  // stop moving, play sound
  return;
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
  for (int i = CALLIBRATE_SEC; i > 0; --i)
    Serial.print(i); Serial.print(".. "); delay(1000);
  for (int i = 0; i < CALIBRATE_NO; ++i) {
    IR_VALUES[0][0] += analogRead(LEFTIR_PIN);
    IR_VALUES[1][0] += analogRead(RIGHTIR_PIN);
    delay(CALIBRATE_DELAY);
  }
  IR_VALUES[0][0] /= CALIBRATE_NO;
  IR_VALUES[1][0] /= CALIBRATE_NO;

  // Max values
  Serial.print("done.\nUNCOVER SENSORS. Calibrating MAX in ");
  for (int i = CALLIBRATE_SEC; i > 0; --i)
    Serial.print(i); Serial.print(".. "); delay(1000);
  for (int i = 0; i < CALIBRATE_NO; ++i) {
    IR_VALUES[0][1] += analogRead(LEFTIR_PIN);
    IR_VALUES[1][1] += analogRead(RIGHTIR_PIN);
    delay(CALIBRATE_DELAY);
  }
  IR_VALUES[0][1] /= CALIBRATE_NO;
  IR_VALUES[1][1] /= CALIBRATE_NO;
  Serial.println("done.\n");

  // Save range
//  Serial.println(IR_VALUES[0][0]);
//  Serial.println(IR_VALUES[0][1]);
//  Serial.println(IR_VALUES[1][0]);
//  Serial.println(IR_VALUES[1][1]);
  IR_VALUES[0][1] -= IR_VALUES[0][0]; // left range
  IR_VALUES[1][1] -= IR_VALUES[1][0]; // right range
}
