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

const int DIR_Pin = 10;
const int STEP_Pin = 11;

//const int Hall_1_Pin = 2;  //up
const int Hall_2_Pin = 3;  //down

int val_1 = 0;
int val_2 = 0;

int speed = 200;

volatile int flag_1 = 0;
volatile int flag_2 = 0;

volatile bool direction = false;  // false = goes up, true = goes down
volatile bool upward = false;
volatile bool downward = false;

int initial_move_increment = 1;
int move_increment = 0;

AccelStepper stepper1(1, STEP_Pin, DIR_Pin);  // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

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
  // CCW // goes up
  /*Serial.println("Rotating CCW / Increment = -1............");
  while (digitalRead(Hall_1_Pin) == 1) {  // No contact with the sensor 1 (1 == no contact / 0 == Contact) (Sensor 1 = ??)
    stepper1.moveTo(initial_move_increment);
    initial_move_increment--;  // CCW
    stepper1.run();
    //Serial.println("Dir B/m_inc= " + String(move_increment));
    delay(1);
  }
  Serial.println("Sensor 1 contacted!");*/

  //stepper1.setCurrentPosition(0);   // Temporal zero position
  //stepper1.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  //stepper1.setAcceleration(100.0);  // Set Acceleration of Stepper
  //initial_move_increment = 1;

  // CW // goes down
  Serial.println("Rotating CW / Increment = +1............");
  while (digitalRead(Hall_2_Pin) == 1) {  // No contact with the sensor 2 (1 == no contact / 0 == Contact) (Sensor 1 = up)
    stepper1.moveTo(initial_move_increment);
    initial_move_increment++;
    stepper1.run();
    delay(1);
  }
  Serial.println("Sensor 2 contacted!");

  stepper1.setCurrentPosition(0);   // Real zero position
  stepper1.setMaxSpeed(1000.0);     // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper1.setAcceleration(500.0);  // Set Acceleration of Stepper

  Serial.println();
  Serial.println();
  Serial.println("End homing process !! ");
}

void setup() {
  Serial.begin(9600);

  //Radio.init();
  Wire.begin();
  radio.setFrequency(93.7);  //On power on go to station 95.2

  delay(5);  // Wait for the motor driver to wake up

  pinMode(Hall_2_Pin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(Hall_2_Pin), eventISR_2, FALLING);

  homeFunction();
  stepper1.setSpeed(speed);
}

void loop() {
  if (move_increment <= -3200) {
    direction = true;
    downward = false;
    Serial.println();
    Serial.println("TOP!!");
    Serial.println();
    delay(5000);
  }
  if (move_increment == 0){
    downward = true;
    direction = false;
    //delay(5000);  
  } 

  stepper1.runSpeedToPosition();
  //delay(10);

  if (direction == true && downward == false) {  // hit the upper sensor. // goes down //cw
    stepper1.moveTo(move_increment);
    stepper1.setSpeed(speed);

    move_increment++;

    Serial.println("CW= " + String(move_increment));
  } else if (direction == false && downward == true) {  // hit the down sensor // goes up // ccw
    stepper1.moveTo(move_increment);
    stepper1.setSpeed(speed);

    move_increment--;

    Serial.println("CCW= " + String(move_increment));  // -3200
  }
}