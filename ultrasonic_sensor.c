#define TIMEOUT 30000
#define WAITING_TIME 1000

#define ULTRASONIC 12
// If you are using PORT 1 of the mCore, the ultrasonic sensor is connected to digital pin 12
// If you are using PORT 2 of the mCore, the ultrasonic sensor is connected to digital pin 10

void setup() {
  // Set up serial communication
  Serial.begin(9600);
}

void loop() {
  // The following code runs repeatedly:
  pinMode(ULTRASONIC, OUTPUT);

  digitalWrite(ULTRASONIC, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC, HIGH);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC, LOW);

  pinMode(ULTRASONIC, INPUT);

  long duration = pulseIn(ULTRASONIC, HIGH, TIMEOUT);

  Serial.print("Duration: ");
  Serial.print(duration);
  Serial.println(" microseconds");

  delay(WAITING_TIME);
}
