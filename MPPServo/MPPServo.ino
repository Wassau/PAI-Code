/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com
  Written by BARRAGAN and modified by Scott Fitzgerald
*********/

#include <Servo.h>
#include <Stepper.h>

// Definitions
#define IN1 33 // ULN2003 Motor Driver Pins
#define IN2 32
#define IN3 35
#define IN4 17
#define servoPin 13 // define pin of data of the servo

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const uint8_t limit = 90;
int pos = 0;    // variable to store the servo position

Servo myservo;  // create servo object to control a servo
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); // initialize the stepper library

void setup() {
  myservo.attach(servoPin);  // attaches the servo on pin 13 to the servo object
  myStepper.setSpeed(5); // set the speed at 5 rpm
  Serial.begin(115200); // initialize the serial port
}

void loop() {

  // step one revolution in one direction:
  Serial.println("clockwise ");
  myStepper.step(stepsPerRevolution);
  delay(10);


  // step one revolution in the other direction:
  Serial.println("counterclockwise");
  myStepper.step(-stepsPerRevolution);
  delay(10); 

}


void servoTwist90NS(){
    for (pos = 0; pos <= limit; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}

void servoTwist90SN(){
    for (pos = limit; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
}
