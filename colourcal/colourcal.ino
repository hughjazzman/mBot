// SEPARATE FILE to calibrate colour
/* TO CHANGE allColourArray in getColour() in main.ino to COLARRs 
after finding values from colourcal.ino */

#include <MeRGBLed.h>

#include "MeMCore.h"
#include "Wire.h"

/* COLOR SENSOR CONSTANTS*/
// Define time delay before the next RGB colour turns ON to allow LDR to stabilize
#define RGBWait 200 //in milliseconds 

// Define time delay before taking another LDR reading
#define LDRWait 10 //in milliseconds 

#define LDR A6   //LDR sensor pin at A6

#define MAXLED 255 // max value of LED RGB values

#define COLOURTHRESHOLD 10 //max deviation from calibrated colour values

// Color Sensor setup - confirmed
MeRGBLed led(7); // pin 7 is the RGB LED (WHY??)

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
float allColourArray[5][3] = {0, 0, 0}; // red,green,yellow,purple,lightblue

char colourStr[3][5] = {"R = ", "G = ", "B = "};

//resulting colour at waypoints (calibration done before start)
int colourRes = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //colour calibration first
  // to do: save values in array
  setBalance();
  for (int i = 0; i < 5; i++) {
    setColours(i);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  colourRes = getColour();
  Serial.print("Your colour is ");
  printColour(colourRes);
  
}

void printColour(int colourRes) {
  String s;
  switch (colourRes){
    case 0:
      s="red";
      break;
    case 1:
      s="green";
      break;
    case 2:
      s="yellow";
      break;
    case 3:
      s="purple";
      break;
    case 4:
      s="light blue";
      break;
    default:
      s="not found";
      break;
  }
  Serial.println(s);
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

int getColour() {
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 3; j++) {
      getColourValues(j);
      delay(RGBWait);
      Serial.println(int(colourArray[j])); //show the value for the current colour LED, which corresponds to either the R, G or B of the RGB code
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
