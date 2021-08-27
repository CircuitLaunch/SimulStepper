# SimulStepper

Arduino sketch using Timer1 interrupts to simultaneously drive multiple stepper motors.

## Introduction
The Arduino provided stepper libraries are not very elegant, and require quite a lot of spaghetti code to get multiple steppers running smoothly simultaneously. The crux of the difficulty is that to keep steppers running smoothly, the microcontroller must be continuously pulsing each pin at strictly timed intervals. Even minor blocks in flow to do other tasks can result in stuttering.
My solution is to use timer1 interrupts to create what is essentially "a preemptive thread" to provide a real-time mechanism for driving the steppers. The TimerOne library is a convenient encapsulation of timer1 interrupts.

## Hardware Setup
Each SD6128 is wired as shown in the following schematic.
![Untitled](https://user-images.githubusercontent.com/6201025/131118331-b902750e-a174-409f-9ee6-dc2882b72b7c.png)
(From [http://files.panucatt.com/datasheets/sd6128_user_guide.pdf](http://files.panucatt.com/datasheets/sd6128_user_guide.pdf))

Of note (thanks to Liam Han for figuring much of this out):

- The RESET and SLEEP pins on the driver are jumped.
- The ENABLE pin must be held LOW to enable operation.
- Use a multimeter to measure the reference voltage across VREF and GND. Use the CURRENT ADUST TRIMMER to set it to half the current rating of the stepper motor.
- The ENABLE, MS1, MS2, MS3, VDD, and GND pins for each of the drivers are connected to the same pins on the µC, i.e., the ENABLE pins for all three boards are connected to pin 10 of the Pro Micro. MS1, MS2, and MS3 for all three boards are connected to pins 3, 4, and 5 respectively. And VDD and GND on all three boards are connected in parallel to the the VCC and GND pins on the Pro Micro.
- +VMOT and GND for all three boards are connected in parallel to one 12V/10A power supply.
- The STEP and DIR pins are connected to pins 6 and 7 for the first driver, 8 and 9 for the second, and 14 and 15 for the third.
- The stepper poles for each motor are connected to 1A, 1B, 2B, and 2A of their respective drivers.
- Some documents suggest dropping a capacitor (min 47 µF) across +VMOT and GND.
- For higher-current applications, attach a heatsink (often included) to the SD6128.
- 
Here's a photograph of the breadboarding.

![IMG_0427](https://user-images.githubusercontent.com/6201025/131118603-5e548f6a-5099-4415-b7cd-1c48b13bb947.jpeg)

## Install the TimerOne Library

Select Tools→Manage Libraries... from the Arduino IDE menu.

![Untitled](https://user-images.githubusercontent.com/6201025/131118739-15be71f6-dcf0-44e1-bbed-1f6fc483069c.png)

In the Library Manager dialog, type "TimerOne" in the filter field. Select and install TimerOne, then click close.

![Untitled](https://user-images.githubusercontent.com/6201025/131118813-8224d189-1b7f-469b-91f1-82fc7644ed4f.png)

## To run the code

Open SimulStepper with the Arduino IDE and flash it to the µC
