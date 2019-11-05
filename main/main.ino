#include "MeMCore.h"
//#include "Wire.h"

/********** Settings **********/
#define LEFTIR_PIN A1
#define RIGHTIR_PIN A2
#define BAND1_PIN A3 // 100-300
#define BAND2_PIN A0 // >3000

// Calibration
bool calibrated = false;
bool busy = false;
#define CALLIBRATE_SEC 3
#define CALIBRATE_NO 5
#define CALIBRATE_DELAY 100
#define BIT_ACC 10



/********** Global Variables **********/
int IR_VALUES[2][2] = {0}; // left-right, minmax



/********** Main Program **********/
void setup() {
    Serial.begin(9600);
    calibrateIR();
}

void loop() {
  // Detect Left-Right Proximity
  IR();
}

// Proximity
void IR() {
  // Position
  // If left > right, going towards right, turn left
  int left = norm(analogRead(LEFTIR_PIN), IR_VALUES[0]);
  int right = norm(analogRead(RIGHTIR_PIN), IR_VALUES[1]);
  left - right

  Serial.print("LEFT: "); Serial.print(left);
  Serial.print("\tRIGHT: "); Serial.println(right);
}



/********** Callibration **********/
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
  IR_VALUES[0][1] -= IR_VALUES[0][0]; // left range
  IR_VALUES[1][1] -= IR_VALUES[1][0]; // right range
}
