/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com
  Written by BARRAGAN and modified by Scott Fitzgerald
  modified by Daniel Delgado 11/06/2022
*********/

#include <Servo.h>
#include <Stepper.h>

// Definitions
#define IN1 26 // ULN2003 Motor Driver Pins
#define IN2 25
#define IN3 33
#define IN4 32
#define servoPin 12 // define pin of data of the servo

// Math constants
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const uint8_t limit = 90;
int pos = 0;    // variable to store the servo position



const int radius = 800;// mm
const int N_rack = 25 * 32; // number of racks by the number of theeth in each rack
const double NperArcLength = N_rack / (PI*radius);
const int unitaryStepPerArcLength = stepsPerRevolution / TWO_PI;
const int stepsPerN = unitaryStepPerArcLength / NperArcLength;

// Home position by default
double elActual = 90;
double azActual = 0;
double elNext = 90;// 42.05
double azNext = 0; // 297.38
double u_carActual = 0;
double u_carNext = 0;
int arcAngleActual = 0;
int arcAngleNext = 0;

// Location
const double latitude = 4.637;
const double longitude = -74.083;

Servo myservo;  // create servo object to control a servo
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); // initialize the stepper library

void setup() {
  myservo.attach(servoPin);  // attaches the servo on pin 13 to the servo object
  myStepper.setSpeed(5); // set the speed at 5 rpm
  Serial.begin(115200); // initialize the serial port
}

void loop() {
  Serial.println("Motor code");

  motorCarMov();
  delay(1000);

  motorArcMov();
  delay(1000);

  showPos();
}


// motors movement
void motorCarMov() {
  myStepper.step(numStepsCar());
  updateValues();
}
void motorArcMov() {
  arcAngleNextCalc();
  (arcAngleActual >= arcAngleNext) ? servoTwist() : servoNegTwist(); //south-north is negative
  updateValues();
}

// arc and servo functions
void arcAngleNextCalc(){
   arcAngleNext = (int)atan2(radius*sin(elNext),radius*cos(elNext)*cos(azNext));
}
void servoNegTwist() {
  for (pos = arcAngleActual; pos > arcAngleNext; pos += 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
  }
}
void servoTwist() {
  for (pos = arcAngleActual; pos < arcAngleNext; pos += 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
  }
}

// car and stepper functions
int numStepsCar() {//number of steps the car has to take
  u_carNext = radius * cos(elNext) * sin(azNext); //
  //d_car = sqrt(pow(radius, 2) * cos(elNext) - pow(radius, 2) * pow(cos(elNext), 2) * pow(cos(azNext), 2)) - u_carActual;
  double theta_u = asin(u_carActual / radius); //
  double theta_u_star = asin(u_carNext / radius);
  double theta_car = theta_u_star - theta_u;
  double C_car = radius * theta_car; //
  int N_car = (int)C_car * NperArcLength; //
  int steps_car = N_car * stepsPerN;
  return steps_car;
}
bool isHome() {
  return ((elActual == 90) && (azActual == 0));
}


// update values
void updateValues(){
  elActual = elNext;
  azActual = elNext;
  arcAngleActual = arcAngleNext;
  u_carActual = u_carNext;
}


// show values in serial monitor
void showPos() {
  Serial.print(F("Elevación actual:"));
  Serial.print(elActual);
  Serial.print(F(" Azimut actual: "));
  Serial.print(azActual);
  Serial.print(F(" Elevación siguiente: "));
  Serial.print(elNext);
  Serial.print(F(" Azimut siguiente: "));
  Serial.print(azNext);
  Serial.print(F(" u_carActual: "));
  Serial.print(u_carActual);
  Serial.print(F(" u_carNext: "));
  Serial.println(u_carNext);
}
