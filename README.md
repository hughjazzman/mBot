# mBot
CG1111 2019/20 Semester 1

## Get Started
1. Build mBot according to `mBot Report.pdf`
2. Compile and Upload `main/main.imo` to mBot

## Feature List
General Movement
- [x] IR Sensor (wk 9-2) – detect walls at side 
- [x] Built-in Ultrasonic Sensor (wk 10-1) – detect wall in front
- [x] Programming – forward movement and readjustment
- [x] Programming - IR Callibration

Waypoint Challenges
- [x] Built-in IR sensor - detect waypoint / black line
- [x] Built-in LDR Detector (wk 9-2) – detect colour on floor & ceiling
- [x] Sound Detector (wk 10-1) x2 – filter & detect frequency of sound
- [x] Programming - turning

Finish
- [x] Speaker (makeblock) – play celebratory tune

## Algorithm
* Loop: Check for waypoint
	* Yes: Check for colour
		* Yes: Move appropriately
		* No: Check for sound
			* Yes: Move apppropriately
			* No: Finish
	* No: Check for front wall (failsafe)
		* Yes: Check side wall - right wall present?
			* Yes: Turn left
			* No: Turn right
		* No: move forward