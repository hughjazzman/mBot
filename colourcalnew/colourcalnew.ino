// SEPARATE FILE to calibrate colour
// Uses CYAN ONLY instead of RGB
/* TO CHANGE allColourArray in getColour() in main.ino to finalColVal 
after finding values from colourcal.ino */

// Final values determined on
// WHIVAL and BLAVAL are white and black values from the light sensor LDR
// using analogRead()
// To be calibrated as close to date
// rest of values can be fixed
#define WHIVAL 620 // value from LDR
#define BLAVAL 530 // value from LDR
#define REDVAL 45 //fixed
#define GREVAL 160 //fixed
#define YELVAL 230 //to check
#define PURVAL 260 //to check
#define BLUVAL 380 //to check
#define BLAVAL 0
#define NUMCOL 6

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

#define COLOURTHRESHOLD 15 //max deviation from calibrated colour values

// Color Sensor setup - confirmed
MeRGBLed led(7); // pin 7 is the RGB LED

/* COLOUR VARIABLES*/

//floats to hold colour arrays
float colourArray = 0;
float whiteArray = WHIVAL; //580, 536
float blackArray = BLAVAL; // 475
float greyDiff = WHIVAL-BLAVAL;
float allColourArray[NUMCOL] = {BLAVAL, REDVAL, GREVAL, YELVAL, PURVAL, BLUVAL }; // black, red,green,yellow,purple,lightblue

//resulting colour at waypoints (calibration done before start)
int colourRes = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //colour calibration first
  // to do: save values in array
  // setBalance();
  //  for (int i = 0; i < 5; i++) {
  //    setColours(i);
  //  }
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
  //scan the white sample.
  //go through one colour at a time, set the maximum reading for each colour -- red, green and blue to the white array
  led.setColor(0, MAXLED, MAXLED);
  led.show();
  delay(RGBWait);
  whiteArray=getAvgReading(5);
  Serial.println(whiteArray);
  delay(RGBWait);
  //done scanning white, time for the black sample.
  //set black balance
  Serial.println("Put Black Sample For Calibration ...");
  delay(5000);     //delay for five seconds for getting sample ready
  led.setColor(0, MAXLED,  MAXLED);
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


void setColours(int colour) {
  switch (colour) {
    case 0: Serial.println("Put Red Sample For Calibration ..."); break;
    case 1: Serial.println("Put Green Sample For Calibration ..."); break;
    case 2: Serial.println("Put Yellow Sample For Calibration ..."); break;
    case 3: Serial.println("Put Purple Sample For Calibration ..."); break;
    case 4: Serial.println("Put Light Blue Sample For Calibration ..."); break;
  }
  delay(5000);
  getColourValues();
  allColourArray[colour] = colourArray;
  delay(RGBWait);
  Serial.println(int(colourArray)); //show the value for the current colour LED, which corresponds to either the R, G or B of the RGB code
  delay(5000);
}

int getColour() {
  int curr, next;
  getColourValues();
  Serial.println(int(colourArray)); //show the value for the current colour LED, which corresponds to either the R, G or B of the RGB code
  for (int i = 0; i < NUMCOL; i++) {
    delay(RGBWait);
	curr = allColourArray[i];
	next = allColourArray[i+1]
    if (i == 6)
      return i;
	else if (colourArray > allColourArray[i] && colourArray < allColourArray[i + 1]) {
		if (colourArray <= (allColourArray[i] + allColourArray[i + 1]) / 2)
			return i;
		else
			return i + 1;
	}
	  
  }
  return -1;
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
