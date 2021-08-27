#include "TimerOne.h"
#include <math.h>

/*
 * Template class to manage multiple stepper motors.
 */
template <int motorCount>
class SimulStepper
{
  public:
    /*
     * Constructor
     *  iStepPins       - an array of motorCount integers specifying the µC pins 
     *                    to which the STEP pins on the drivers are connected
     *  iDirPins        - an array of motorCount integers specifying the uC pins 
     *                    to which the DIR pins on the drivers are connecteds
     *  iTickFrequency  - the frequency with which you intend to call the tick() 
     *                function
     *  iSteps          - the numbers of steps in a revolution (200 is typical)
     *  iMS1Pin         - the µC pin connected to the MS1 pins on the drivers
     *  iMS2Pin         - the µC pin connected to the MS2 pins on the drivers
     *  iMS3Pin         - the µC pin connected to the MS3 pins on the drivers
     *  iEnablePin      - the uC pin connected to the ENABLE pins on the drivers
     */
    SimulStepper(int *iStepPins, int *iDirPins, int iTickFrequency, int iSteps=200,
    int iMS1Pin=3, int iMS2Pin=4, int iMS3Pin=5, int iEnablePin=10)
    : tickFrequency(iTickFrequency), steps(iSteps), 
      ms1Pin(iMS1Pin), ms2Pin(iMS2Pin), ms3Pin(iMS3Pin), enablePin(iEnablePin) {
      
      // All motors share the same MS and ENABLE pins
      pinMode(ms1Pin, OUTPUT);
      pinMode(ms2Pin, OUTPUT);
      pinMode(ms3Pin, OUTPUT);
      pinMode(enablePin, OUTPUT);
      
      // Polulu DRV8825 and compatible boards require the enable pin to be held 
      // low to enable
      digitalWrite(enablePin, LOW);

      // Iterate through and initialize the array of config and state structures
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

    /*
     * Must be called as regularly and frequently as possible
     */
    void tick() {
      int i = motorCount;
      
      // Iterate through the motors
      while(i--) {
        
        // If the speed is not zero
        if(motors[i].stepTicks) {
          
          // If the tickCount has reached zero
          if(!motors[i].tickCount) {
            
            // Set the microstepping level
            digitalWrite(ms1Pin, motors[i].ms1);
            digitalWrite(ms2Pin, motors[i].ms2);
            digitalWrite(ms3Pin, motors[i].ms3);
            
            // Set the direction
            digitalWrite(motors[i].dirPin, motors[i].dir);
            
            // Pulse the step pin
            digitalWrite(motors[i].stepPin, HIGH);
            unsigned long ustart = micros();
            while((micros() - ustart) < 1);
            digitalWrite(motors[i].stepPin, LOW);
            
            // Reset the tickCount
            motors[i].tickCount = motors[i].stepTicks;

          // Otherwise decrement the tickCount
          } else motors[i].tickCount--;
        }
      }
    }

    /*
     * Function to set the revolutions per minute and the microstep settings for a motor
     */
    void setRPM(int iMotor, float iRPM, int iMicrosteps = 1) {

      // Precalculate everything to avoid disabling interrupts for too long.
      
      // The base2 log of the microsteps will give us the flags required to set the MS pins
      unsigned int flags = logf(float(iMicrosteps)) / logf(2.0);

      // The steps per second is 
      //     (the number of steps per revolution)
      //   * (the absolute revolutions per minute) / 60
      //   * (the microsteps)
      float stepsPerSecond = 
             float(steps)
           * fabsf(iRPM) * 0.01666666667
           * iMicrosteps;

      // The number of ticks between pulses is
      //     (the frequency) / (the steps per second)
      unsigned int stepTicks = 
             (unsigned int)(float(tickFrequency) / stepsPerSecond);

      // Test the microstep flags
      unsigned int ms1 = (flags & 1) != 0;
      unsigned int ms2 = (flags & 2) != 0;
      unsigned int ms3 = (flags & 4) != 0;

      // Establish the direction
      unsigned int dir = iRPM < 0;

      // Ensure that an interrupt doesn't preempt us while we're setting these 
      // shared values
      noInterrupts();

      // Store the calculated values for the given motor
      motors[iMotor].ms1 = ms1;
      motors[iMotor].ms2 = ms2;
      motors[iMotor].ms3 = ms3;
      motors[iMotor].dir = dir;
      motors[iMotor].stepTicks = stepTicks;

      // Reenable interrupts
      interrupts();
    }
    
  protected:
    /*
     * Structure to store motor configuration an state
     */
    typedef struct Motor {
      unsigned int stepPin;
      unsigned int dirPin;
      unsigned int stepTicks;
      unsigned int ms1, ms2, ms3;
      unsigned int dir;
      unsigned long tickCount;
    } Motor;

    /*
     * Variables to store configuration common to all motors
     */
    unsigned int tickFrequency;
    unsigned int steps;
    unsigned int ms1Pin;
    unsigned int ms2Pin;
    unsigned int ms3Pin;
    unsigned int enablePin;

    /*
     * Array of motors
     */
    Motor motors[motorCount];
}; // End of template class SimulStepper

/*
 * Create a manager for 3 stepper motors.
 * 
 * For testing, I am using a Pro Micro (an Arduino Leonardo clone), and three 
 * independent SD6128 driver boards.
 *
 * I will be calling the tick() function at 10 kHz.
 */
int stepPins[3] = { 6, 8, 14 };
int dirPins[3] = { 7, 9, 15 };
SimulStepper<3> manager(stepPins, dirPins, 10000);

/*
 * Timer1 interrupt handler
 */
void timer1Tick() {
  // Pass control to the manager
  manager.tick();
}

/*
 * This is where it all starts
 */
void setup() {
  // Set the timer to generate interrupts every 100 µs (i.e. at 10000 Hz)
  Timer1.initialize(100);

  // Attach the handler to the interrupt
  Timer1.attachInterrupt(timer1Tick);
}

/*
 * Some variables to do some acceleration and deceleration on "the main thread"
 */
int state = 0;
int max0 = 100, max1 = max0-1;
int rpm;

/*
 * The "main" loop
 */
void loop() {
  
  // Accelerate the motors from 0 to 100 at 1/8 microstep resolution
  for(int i = 0; i < max0; i++) {
    manager.setRPM(0, i, 8);
    manager.setRPM(1, i, 8);
    manager.setRPM(2, i, 8);
    delay(17);
  }

  // Decelerate the motors from 100 to 0 at 1/8 microstep resolution
  for(int i = 0; i < max0; i++) {
    rpm = max1 - i;
    manager.setRPM(0, rpm, 8);
    manager.setRPM(1, rpm, 8);
    manager.setRPM(2, rpm, 8);
    delay(17);
  }

  // Accelerate the motors from 0 to -100 at 1/8 microstep resolution
  for(int i = 0; i < max0; i++) {
    manager.setRPM(0, -i, 8);
    manager.setRPM(1, -i, 8);
    manager.setRPM(2, -i, 8);
    delay(17);
  }

  // Decelerate the motors from -100 to 0 at 1/8 microstep resolution
  for(int i = 0; i < max0; i++) {
    rpm = i - max1;
    manager.setRPM(0, rpm, 8);
    manager.setRPM(1, rpm, 8);
    manager.setRPM(2, rpm, 8);
    delay(17);
  }
}
