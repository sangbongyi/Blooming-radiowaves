/* 
  Blooming radiowaves project
  Copyright (c) 2024 Sangbong Lee <sangbong@me.com>
  
  * Stepper motor controller that rotates an aluminum tube to fold/unfold the membrane antenna of the device. 
  * This code allows the Arduino-every board to operate a stepper motor to change the shape of the folding membrane of the antenna. 
  * While it folds, it changes the receiving radio frequency by changing the exposed surface of copper trace on the membrane.
  * It also activates the TDA5767 radio module that receives an FM signal from the antenna.

  This work is licensed under the Creative Commons Attribution 4.0 International License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/

#include <Arduino.h>
#include <Wire.h>
#include <AccelStepper.h>
#include <TEA5767Radio.h>

TEA5767Radio radio = TEA5767Radio();
AccelStepper stepper1(1, STEP_Pin, DIR_Pin);  // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

// Stepper motor controller pins
const int DIR_Pin = 10;
const int STEP_Pin = 11;

// Stepper motor position flags
volatile bool direction = false;  // false = goes up, true = goes down
volatile bool upward = false;
volatile bool downward = false;

// Stepper motor variables
int initial_move_increment = 1;
int move_increment = 0;
int speed = 200;

// Hall sensor pin
const int Hall_2_Pin = 3;  //down

// Hardware interrupt
// Change flag variables to reverse the direction
void eventISR_2() {
  //upward = false;
  downward = true;
  direction = false;
}

void homeFunction() {
  // Home function set up
  stepper1.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper1.setAcceleration(100.0);  // Set Acceleration of Stepper

  Serial.println("Start homing process..............");
  Serial.println();
  Serial.println();

  // CW // goes down
  Serial.println("Rotating CW / Increment = +1............");
  while (digitalRead(Hall_2_Pin) == 1) {  // No contact with the sensor 2 (1 == no contact / 0 == Contact)
    stepper1.moveTo(initial_move_increment);
    initial_move_increment++;
    stepper1.run();
    delay(1);
  }
  Serial.println("HallSensor contacted!");

  stepper1.setCurrentPosition(0);   // Set the current position as the start position
  stepper1.setMaxSpeed(1000.0);     // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper1.setAcceleration(500.0);  // Set Acceleration of Stepper

  Serial.println();
  Serial.println();
  Serial.println("End homing process !! ");
}

void setup() {
  // Set serial communication for debugging
  Serial.begin(9600);
  // Set I2C communication for the radio module
  Wire.begin();
  // On power-on go to station FM93.7
  radio.setFrequency(93.7);  
  delay(5);  // Wait for the motor driver to wake up
  // Set the hall sensor pin (HIGH == No contact // LOW == Contact)
  pinMode(Hall_2_Pin, INPUT_PULLUP);
  // Set hardware interrupt
  attachInterrupt(digitalPinToInterrupt(Hall_2_Pin), eventISR_2, FALLING);
  // Set the stepper motor to its home position
  homeFunction();
  stepper1.setSpeed(speed);
}

void loop() {
  // Reverse rotation direction when the membrane reaches the top part of the frame
  // Fully unfolded
  if (move_increment <= -3200) {
    direction = true;
    downward = false;
    Serial.println();
    Serial.println("TOP!!");
    Serial.println();
    delay(5000);
  }
  // Fully folded
  if (move_increment == 0){
    downward = true;
    direction = false;
  } 

  // Set the stepper to runmode
  stepper1.runSpeedToPosition();
  
  // Run the stepper motor by its fixed speed
  if (direction == true && downward == false) {  
    stepper1.moveTo(move_increment);
    stepper1.setSpeed(speed);
    move_increment++; // Increase step until it reaches the target. (Rotation CW / Goes down)

    Serial.println("CW= " + String(move_increment));
  } 
  else if (direction == false && downward == true) {  
    stepper1.moveTo(move_increment);
    stepper1.setSpeed(speed);
    move_increment--; // Decrease step until it reaches the target. (Rotation CCW / Goes up

    Serial.println("CCW= " + String(move_increment));  // -3200
  }
}