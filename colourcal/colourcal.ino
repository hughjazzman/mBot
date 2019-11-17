// SEPARATE FILE to calibrate colour

#include <MeMCore.h>
#include "Wire.h"

/********** Settings **********/
#define LDR             A6              // LDR sensor pin at A6
MeRGBLed led(7);                        // RGB LED at pin 7

// Values determined on 14 Nov 19
// LDR Values for White/Black before normalisation
#define WHI_VAL         {375, 335, 380}         
#define BLA_VAL         {255, 217, 243}
#define GRE_VAL         {116, 108, 130}
// Normalised RGB Values for Colours
#define RED_ARR         {185, 35, 35}             
#define GRE_ARR         {45, 100, 60}
#define YEL_ARR         {255, 175, 100}
#define PUR_ARR         {155, 150, 200}
#define BLU_ARR         {175, 240, 240}
#define BLA_ARR         {0}
#define NUMCOL          6               // No. of colours to detect

// Calibration
// Define time delay before the next RGB colour turns ON to allow LDR to stabilize
#define RGBWait         200             // in milliseconds 
// Define time delay before taking another LDR reading
#define LDRWait         10              // in milliseconds 
#define MAXLED          255             // max value of LED RGB values
#define MIN_DIST        5000
#define CAL_NO          50

/********** Global Variables **********/
// Placeholders for colour detected
int red = 0;
int green = 0;
int blue = 0;

// Arrays to hold colour values
float colourArray[] = {0};
int whiteArray[] = WHI_VAL;
int blackArray[] = BLA_VAL;//{315, 265, 305};
int greyDiff[] = GRE_VAL;
static int allColourArray[NUMCOL][3] = {BLA_ARR, RED_ARR, GRE_ARR, YEL_ARR, PUR_ARR, BLU_ARR};

char colourStr[3][5] = {"R = ", "G = ", "B = "};

// Value corresponding to each colour
int colourRes;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setBalance();
}

void loop() {
  // put your main code here, to run repeatedly:
  // This code will automatically print the RGB Values of the current object.
  colourRes = getColour();
  Serial.print("Your colour is ");
  printColour(colourRes);
  
}

void printColour(int colourRes) {
  String s;
  switch (colourRes){
    case 0: s="black"; break;
    case 1: s="red"; break;
    case 2: s="green"; break;
    case 3: s="yellow"; break;
    case 4: s="purple"; break;
    case 5: s="light blue"; break;
    default: s="not found"; break;
  }
  Serial.println(s);
}

/* COLOUR FUNCTIONS */
// setBalance       - calibrate white and black values
// getColourValues  - gets colour values of a given sample
// getColour        - get colour from the default colours
// getAvgReading    - gets direct reading from LDR

void setBalance() {
  //set white balance
  Serial.println("Put White Sample For Calibration ...");
  delay(5000);           //delay for five seconds for getting sample ready
  //digitalWrite(LED,LOW); //Check Indicator OFF during Calibration
  //scan the white sample.
  //go through one colour at a time, set the maximum reading for each colour -- red, green and blue to the white Array
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
    whiteArray[i] = getAvgReading(CAL_NO);
    Serial.println(whiteArray[i]);
    delay(RGBWait);
  }
  led.setColor(0, 0, 0);
  led.show();
  //done scanning white, time for the black sample.

  //set black balance
  Serial.println("Put Black Sample For Calibration ...");
  delay(5000);     //delay for five seconds for getting sample ready
  //go through one colour at a time, set the minimum reading for red, green and blue to the black Array
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
    blackArray[i] = getAvgReading(CAL_NO);
    Serial.println(blackArray[i]);
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
  int idx=-1,min_dist=MIN_DIST;
  long long curr_dist;

  // Gets RGB values for current colour. This prints the RGB values of the current object.
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
  colourArray[rgb] = getAvgReading(CAL_NO);
  //the average reading returned minus the lowest value divided by the maximum possible range, multiplied by 255 will give a value between 0-255, representing the value for the current reflectivity (i.e. the colour LDR is exposed to)
  colourArray[rgb] = (colourArray[rgb] - blackArray[rgb]) / (greyDiff[rgb]) * MAXLED;
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
