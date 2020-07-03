#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible


#define RIGHT_TAPE_SENSOR PA5 //ANALOG
#define LEFT_TAPE_SENSOR PA4 //ANALOG

#define TAPE_SENSOR_POT PA0 //ANALOG

#define CORRECTING_SPEED_POT PB0 //ANALOG
#define DISPLAY_BUTTON PA3 //DIGITAL

#define RIGHT_MOTOR_FORWARD PB_9 //PWM
#define RIGHT_MOTOR_REVERSE PA_1 //PWM

#define LEFT_MOTOR_FORWARD PB_8 //PWM
#define LEFT_MOTOR_REVERSE PA_2 //PWM

#define RIGHT 0
#define LEFT 1

#define PWM_FREQUENCY 1000

#define STOP 0
#define FULL_SPEED 512
#define REVERSE_SPEED 512*0.35

#define FORWARD 1
#define REVERSE 0


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void readCorrectionSpeed();
void correctDirection();
void printMessage(int, bool);
void printMessage(String, bool);
void refreshDisplay();
void readTapeSensors();
void turn(int);
void turnWithReverse(int);
void driveStraight(int);
void driveMotor(int, int, int);

bool rightReflectance;
bool leftReflectance;
unsigned int tapeSensorThreahold;

int lastSideOnTape = RIGHT;

unsigned int correctingSpeed;


void setup() {
  // initialize LED digital pin as an output and set on
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
  pinMode(RIGHT_MOTOR_REVERSE, OUTPUT);

  pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
  pinMode(LEFT_MOTOR_REVERSE, OUTPUT);

  pinMode(DISPLAY_BUTTON, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  }

void loop() {
  readCorrectionSpeed();
  readTapeSensors();
  if (digitalRead(DISPLAY_BUTTON))
  {
    correctDirection();
  }
  else{
    driveStraight(STOP);
    refreshDisplay();
    printMessage(tapeSensorThreahold, true);
    printMessage(analogRead(RIGHT_TAPE_SENSOR), false);
    printMessage("   ", false);
    printMessage(rightReflectance, true);
    printMessage(analogRead(LEFT_TAPE_SENSOR), false);
    printMessage("   ", false);
    printMessage(leftReflectance, true);
    printMessage(analogRead(CORRECTING_SPEED_POT), false);
    printMessage("   ", false);
    printMessage(correctingSpeed *100 / 512, true);
    display.display();
  }
}

void readCorrectionSpeed(){
  correctingSpeed = map(analogRead(CORRECTING_SPEED_POT), 0, 1023, 0, FULL_SPEED);
}

void correctDirection(){
  if(rightReflectance && leftReflectance){
    driveStraight(correctingSpeed);
    //printMessage("Straight", true);
  }
  else if (rightReflectance){
    turn(RIGHT);
    //turnWithReverse(RIGHT);
    //printMessage("Turn Right", true);
  }
  else if (leftReflectance){
    turn(LEFT);
    //turnWithReverse(LEFT);
    //printMessage("Turn Left", true);
  }
  else{
    turnWithReverse(lastSideOnTape);
    //turn(lastSideOnTape);
    // printMessage("OFF");
    // refreshDisplay();
    //printMessage(leftReflectance, false);
    //printMessage(rightReflectance, false);
    //printMessage("W/ Reverse", false);
    //printMessage(lastSideOnTape, true);
    
  }
  //display.display();
}

void printMessage(int message, bool newline){
  printMessage(String(message), newline);
}

void printMessage(String message, bool newline){
   if (newline){
    display.println(message);
  }
  else{
    display.print(message);
  }
}

void refreshDisplay(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
}

void readTapeSensors(){
  //refreshDisplay();
  tapeSensorThreahold = analogRead(TAPE_SENSOR_POT);

  rightReflectance = analogRead(RIGHT_TAPE_SENSOR) > tapeSensorThreahold;
  leftReflectance = analogRead(LEFT_TAPE_SENSOR) > tapeSensorThreahold;

  if (rightReflectance && !leftReflectance){
    lastSideOnTape = RIGHT;
    
    //printMessage("Right Last", true);
  }
  else if (leftReflectance && !rightReflectance){
    lastSideOnTape = LEFT;
    //printMessage("Left Last", true);
  }
}

void turn(int side){
  if (side == RIGHT) {
    driveMotor(LEFT, correctingSpeed, FORWARD);
    driveMotor(RIGHT, STOP, FORWARD);
  }
  else if (side == LEFT) {
    driveMotor(RIGHT, correctingSpeed, FORWARD);
    driveMotor(LEFT, STOP, FORWARD);
  }
}

void turnWithReverse(int side){
  if (side == RIGHT) {
    driveMotor(LEFT, correctingSpeed, FORWARD);
    driveMotor(RIGHT, REVERSE_SPEED, REVERSE);
  }
  else if (side == LEFT) {
    driveMotor(RIGHT, correctingSpeed, FORWARD);
    driveMotor(LEFT, REVERSE_SPEED, REVERSE);
  }
}

void driveStraight(int speed){
  driveMotor(RIGHT, speed, FORWARD);
  driveMotor(LEFT, speed, FORWARD);
}

void driveMotor(int motorSide, int speed, int direction){
  if (motorSide == RIGHT) {
    if (direction == FORWARD) {
      pwm_start(RIGHT_MOTOR_REVERSE, PWM_FREQUENCY, 0, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(RIGHT_MOTOR_FORWARD, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
    else if (direction == REVERSE){
      // refreshDisplay();
      // printMessage("Right Reverse", true);
      // display.display();
      pwm_start(RIGHT_MOTOR_FORWARD, PWM_FREQUENCY, 0, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(RIGHT_MOTOR_REVERSE, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
  }
  else if (motorSide == LEFT) {
    if (direction == FORWARD) {
      pwm_start(LEFT_MOTOR_REVERSE, PWM_FREQUENCY, 0, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(LEFT_MOTOR_FORWARD, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
    else if (direction == REVERSE){
      // refreshDisplay();
      // printMessage("Left Reverse", true);
      // display.display();
      pwm_start(LEFT_MOTOR_FORWARD, PWM_FREQUENCY, 0, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(LEFT_MOTOR_REVERSE, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
  }
}
