#include <Stepper.h>
/*
 * Interrupciones de los finales de carrera.
 */

# define Limcarro 25
# define Limarco 26  
//Stepper Nema23(STEPS_PER_REV, 8, 9, 10, 11);

// DC MOTOR PINS
#define ENCB 22
#define PWM_pin 21
#define dirmas 19
#define dirmenos 18
// PWM Properties.
const int freq = 30000;
const int Channel = 0;
const int resolution = 8;
int dutyCycle = 200;
int pos = 0; // Posición encoder

// Definitions Step Motor
#define Enable 17 // Motor Driver Pins
#define DIRSTPM 16
#define PULSE 4
#define IN4 32

const int STEPS_PER_REV = 200;
int dirPin = 0;


//Stepper Nema23(STEPS_PER_REV, 8, 9, 10, 11);
void setup() {
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(Limcarro),Limitcarro,RISING);
  attachInterrupt(digitalPinToInterrupt(Limcarro),Limitarco,RISING);
  ledcSetup( Channel, freq, resolution); // set the PWM parameters 
  ledcAttachPin(PWM_pin, Channel); // assign the Pin for PWM
  


}

void loop() {
  // put your main code here, to run repeatedly:

}

void Limitcarro (){
  bool limite_carro = digitalRead(Limcarro);
  if (limite_carro) {
    ledcWrite(Channel, 0); 
  }
  
}

void Limitarco (){
  bool limite_arco = digitalRead(Limcarro);
  if (limite_arco) {
    digitalWrite(PULSE,LOW);
    digitalWrite(Enable, HIGH);
  }
}
void Setmotor(int dirPin, int Duty){
  if (dirPin == 1){
    // Set motor direction clockwise
      for(int x = 0; x < STEPS_PER_REV ; x++) { // one rotation
        digitalWrite(PULSE,LOW);
        delayMicroseconds(Duty);
        digitalWrite(PULSE,HIGH);
        delayMicroseconds(Duty);
      }
  }
      else {
   // Set motor direction counterclockwise
      for(int x = 0; x < STEPS_PER_REV ; x++) { // one rotation
        digitalWrite(PULSE,HIGH);
        delayMicroseconds(Duty);
        digitalWrite(PULSE,LOW);
        delayMicroseconds(Duty);
      }
  }
 }
