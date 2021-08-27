#include "TimerOne.h"
#include <math.h>

template <int motorCount>
class SimulStepper {
  public:
    SimulStepper(int *iStepPins, int *iDirPins, int iTickFrequency, int iSteps=200, int iMS1Pin=3, int iMS2Pin=4, int iMS3Pin=5, int iEnablePin=10)
    : tickFrequency(iTickFrequency), steps(iSteps), ms1Pin(iMS1Pin), ms2Pin(iMS2Pin), ms3Pin(iMS3Pin), enablePin(iEnablePin) {
      pinMode(17, OUTPUT);
      pinMode(ms1Pin, OUTPUT);
      pinMode(ms2Pin, OUTPUT);
      pinMode(ms3Pin, OUTPUT);
      pinMode(enablePin, OUTPUT);
      digitalWrite(enablePin, LOW);
      int i = motorCount;
      while(i--) {
        pinMode(iStepPins[i], OUTPUT);
        pinMode(iDirPins[i], OUTPUT);
        motors[i].stepPin = iStepPins[i];
        motors[i].dirPin = iDirPins[i];
        motors[i].stepTicks = 0;
        motors[i].dir = 0;
        motors[i].tickCount = 0;
      }
    }
    
    void tick() {
      int i = motorCount;
      while(i--) {
        if(motors[i].stepTicks) {
          if(!motors[i].tickCount) {
            digitalWrite(17, HIGH);
            digitalWrite(ms1Pin, motors[i].ms1);
            digitalWrite(ms2Pin, motors[i].ms2);
            digitalWrite(ms3Pin, motors[i].ms3);
            digitalWrite(motors[i].dirPin, motors[i].dir);
            digitalWrite(motors[i].stepPin, HIGH);
            unsigned long ustart = micros();
            while((micros() - ustart) < 1);
            digitalWrite(motors[i].stepPin, LOW);
            motors[i].tickCount = motors[i].stepTicks;
            digitalWrite(17, LOW);
          } else motors[i].tickCount--;
        }
      }
    }
    
    void setRPM(int iMotor, float iRPM, int iMicrosteps = 1) {
      unsigned int microsteps = logf(float(iMicrosteps)) / logf(2.0);
      float stepsPerSecond = float(steps) * fabsf(iRPM) * 0.01666666667 * iMicrosteps;
      unsigned int stepTicks = (unsigned int)(float(tickFrequency) / stepsPerSecond);
      unsigned int ms1 = (microsteps & 1) != 0;
      unsigned int ms2 = (microsteps & 2) != 0;
      unsigned int ms3 = (microsteps & 4) != 0;
      Serial.print("microsteps = "); Serial.println(microsteps);
      Serial.print("ms1 = "); Serial.println(ms1);
      Serial.print("ms2 = "); Serial.println(ms2);
      Serial.print("ms3 = "); Serial.println(ms3);
      unsigned int dir = iRPM < 0;
      noInterrupts();
      motors[iMotor].ms1 = ms1;
      motors[iMotor].ms2 = ms2;
      motors[iMotor].ms3 = ms3;
      motors[iMotor].dir = dir;
      motors[iMotor].stepTicks = stepTicks;
      interrupts();
    }
    
  protected:
    typedef struct Motor {
      unsigned int stepPin;
      unsigned int dirPin;
      unsigned int stepTicks;
      unsigned int ms1, ms2, ms3;
      unsigned int dir;
      unsigned long tickCount;
    } Motor;
    
    unsigned int tickFrequency;
    unsigned int steps;
    unsigned int ms1Pin;
    unsigned int ms2Pin;
    unsigned int ms3Pin;
    unsigned int enablePin;

    Motor motors[motorCount];
};

int stepPins[3] = { 6, 8, 14 };
int dirPins[3] = { 7, 9, 15 };

SimulStepper<3> stepper(stepPins, dirPins, 10000);

void timer1Tick() {
  stepper.tick();
}

void setup() {
  Serial.begin(9600);
  Timer1.initialize(100);
  Timer1.attachInterrupt(timer1Tick);
}

int state = 0;
int max0 = 100, max1 = max0-1;
int rpm;
void loop() {
  for(int i = 0; i < max0; i++) {
    stepper.setRPM(0, i, 8);
    stepper.setRPM(1, i, 8);
    stepper.setRPM(2, i, 8);
    delay(17);
  }
  for(int i = 0; i < max0; i++) {
    rpm = max1 - i;
    stepper.setRPM(0, rpm, 8);
    stepper.setRPM(1, rpm, 8);
    stepper.setRPM(2, rpm, 8);
    delay(17);
  }
  for(int i = 0; i < max0; i++) {
    stepper.setRPM(0, -i, 8);
    stepper.setRPM(1, -i, 8);
    stepper.setRPM(2, -i, 8);
    delay(17);
  }
  for(int i = 0; i < max0; i++) {
    rpm = i - max1;
    stepper.setRPM(0, rpm, 8);
    stepper.setRPM(1, rpm, 8);
    stepper.setRPM(2, rpm, 8);
    delay(17);
  }
}
