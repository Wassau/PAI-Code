//======================================================================================================================
// SolarCalculator Library for Arduino example sketch: SolarTrackingTimeLib.ino
//
// Monitor the Sun's position in the sky for any location on Earth.
//
// Tested with Arduino IDE 1.8.19 and Arduino Uno
//======================================================================================================================

#include <SolarCalculator.h>
#include <TimeLib.h>
#include <Servo.h>
#include <Stepper.h>

// Definitions of the pins for motors
#define IN1 33 // ULN2003 Motor Driver Pins
#define IN2 32
#define IN3 35
#define IN4 17
#define servoPin 13 // define pin of data of the servo

// Location
const double latitude = 4.637;
const double longitude = -74.083;
int utc_offset = (int)floor(longitude / 15);
uint16_t yearH = 2022;             // Calendar year (1901-2099)
uint8_t monthH = 6;               // Calendar month (1-12)
uint8_t dayH = 1;                 // Calendar day (1-31)
uint8_t hourH = 6;            // the hour now  (0-23)
uint8_t minuteH = 0;          // the minute now (0-59)
uint8_t secondH = 0;          // the second now (0-59)

// arc and car variables and constans
int stepArc = 1;
int stepCar = 1;
double actualPosArc;
double nextPosArc;
double actualPosCar;
double nextPosCar;

// motors variables and constants
const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const uint8_t limit = 90;
int pos = 0;    // variable to store the servo position

// counters
uint8_t hourCount = 0;

// boolean variables
bool isAutomatic = false;
bool isManual = false;
bool automaticDayBoxChecked = false;
bool eventBtnN = false, eventBtnS = false, eventBtnW = false, eventBtnE = false;

// time constants
uint8_t sunRise_offset = 5; // 5 minute of offset for the sun rise
uint8_t sunSet_offset = 5; // 5 minute of offset for the sun rise

// Refresh interval, in seconds
int interval = 1;

Servo myservo;  // create servo object to control a servo
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4); // initialize the stepper library

time_t utc;
double az, el;

void setup()
{
  Serial.begin(9600);

  // Set system time to compile time
  //setTime(toUtc(compileTime()));

  // Set time manually (hr, min, sec, day, mo, yr)
  myservo.attach(servoPin);  // attaches the servo on the defined pin to the servo object
  myStepper.setSpeed(5); // set the speed at 5 rpm
  setTime(correctedHour(hourH), minuteH, secondH, dayH, monthH, yearH);
}

void loop()
{
  static unsigned long next_millis = 0;



  // At every interval
  if (millis() > next_millis)
  {
    Serial.print(F("GMT: "));
    Serial.print(utc_offset);
    Serial.print(F(" Day: "));
    Serial.print(dayH);
    Serial.print(F(" Time: "));
    Serial.print(hourH);
    Serial.print(F(":"));
    Serial.print(minuteH);
    Serial.print(F(":"));
    Serial.println(secondH);

    if (validHour(hourH)) {
      utc = now();
      //double az, el;

      // Calculate the solar position, in degrees
      calcHorizontalCoordinates(utc, latitude, longitude, az, el);

      // Print results
      Serial.print(F("Az: "));
      Serial.print(az);
      Serial.print(F("°  El: "));
      Serial.print(el);
      Serial.println(F("°"));
    } else {
      Serial.println(F("Invalid hour"));
    }

    hourCount++;
    hourH = + hourCount;
    setTime(correctedHour(hourH), minuteH, secondH, dayH, monthH, yearH);
    next_millis = millis() + interval * 1000L;// the L in 1000L means declares the value as Long
  }
}


bool validHour(int hourH) {
  return (hourH >= 6 && hourH <= 18) ? true : false;
}

// for some reason the values of the "az" and the "el" differ from the
// real values they shoud have in 5 hours, this is a correction for utc calculations
int correctedHour(int utcHour) {
  return utcHour - utc_offset;
  //return utcHour;
}

// this is a correction for printing the hour it should have
int correctedHourPrint(int utcHour) {
  return utcHour + utc_offset;
}

time_t toUtc(time_t local)
{
  return local - utc_offset * 3600L;
}

// Code from JChristensen/Timezone Clock example
time_t compileTime()
{
  const uint8_t COMPILE_TIME_DELAY = 8;
  const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char chMon[4], *m;
  tmElements_t tm;

  strncpy(chMon, compDate, 3);
  chMon[3] = '\0';
  m = strstr(months, chMon);
  tm.Month = ((m - months) / 3 + 1);

  tm.Day = atoi(compDate + 4);
  tm.Year = atoi(compDate + 7) - 1970;
  tm.Hour = atoi(compTime);
  tm.Minute = atoi(compTime + 3);
  tm.Second = atoi(compTime + 6);
  time_t t = makeTime(tm);
  return t + COMPILE_TIME_DELAY;
}

// *****************************************
// ********* FUNICIONES EN PROCESO *********
// *****************************************

//void automaticPointMov() {
//  if (isAutomatic) {
//    while (true) {
//      if (comparisonActualNext(actualPosArc, nextPosArc)) {
//        movMotorArc();
//      }
//      if (comparisonActualNext(actualPosCar, nextPosCar)) {
//        movMotorCar();
//      }
//      if (comparisonActualNext(actualPosArc, nextPosArc) && comparisonActualNext(actualPosCar, nextPosCar)) {
//        break;
//      }
//    }
//  }
//}
//
//void manualMov() {
//  if (eventBtnN) {
//    actualPosArc = + stepArc;
//    movMotorArc();
//    eventBtnN = !eventBtnN;
//  }
//  if (eventBtnS) {
//    actualPosArc = - stepArc;
//    movMotorArc();
//    eventBtnS = !eventBtnS;
//  }
//  if (eventBtnW) {
//    actualPosCar = - stepCar;
//    movMotorCar();
//    eventBtnW = !eventBtnW;
//  }
//  if (eventBtnE) {
//    actualPosCar = + stepCar;
//    movMotorCar();
//    eventBtnE = !eventBtnE;
//  }
//}
//
//// simulate day for every hour
//void automaticDayMov() {
//  if (automaticDayBoxChecked) {
//    // disableTime(); // disable time input in UI
//    setAtSunRise();
//    minuteH = 0;
//    for (uint8_t i = 1; i < 12; i++) {
//      hourH = + i;
//      setAzEl();
//      nextPosArc = az * 100.0;
//      nextPosCar = el * 100.0;
//      automaticPointMov();
//    }
//    setAtSunSet();
//  }
//}
//
//void movMotorArc(){
//  if(actualPos>latitude){
//    
//  }
//}
//
//void servoTwist90NS(){
//    for (pos = 0; pos <= limit; pos += 1) { // goes from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    myservo.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(15);                       // waits 15ms for the servo to reach the position
//  }
//}
//
//void servoTwist90SN(){
//    for (pos = limit; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
//    myservo.write(pos);              // tell servo to go to position in variable 'pos'
//    delay(15);                       // waits 15ms for the servo to reach the position
//  }
//}
//
//
//void movMotorCar(){
//  
//}
//
//void setAtSunRise() {
//  hourH = 6;
//  minuteH = sunRise_offset;
//  setAzEl();
//  nextPosArc = az * 100.0;
//  nextPosCar = el * 100.0;
//  automaticPointMov()
//}
//
//void setAtSunSet() {
//  hourH = 17;
//  minuteH = 59 - sunSet_offset; // the sunSet_offset starts to count from minute 59
//  setAzEl();
//  nextPosArc = az * 100.0;
//  nextPosCar = el * 100.0;
//  automaticPointMov()
//}
//
//// does the calculations for the azimut and the elevation
//void setAzEl() {
//  setTime(correctedHour(hourH), minuteH, secondH, dayH, monthH, yearH);
//  utc = now();
//  calcHorizontalCoordinates(utc, latitude, longitude, az, el);
//}
//
//bool updatePosLong(){
//  if(validPosLong()){
//    (nextPosArc>actualPosArc) ? 
//  }
//}
//bool updatePosLat(){
//  if(validPosLat()){
//    (latitude
//  }
//}
//
//
//bool comparisonActualNext(int actualPos, int nextPos) {
//  return (actualPos >= nextPos || actualPos <= nextPos)
//}
//
