#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible

#define DRIVE_SPEED_POT PB1 //ANALOG
#define CORRECTING_SPEED_POT PB0 //ANALOG

#define ROTOR_MOTOR_FORWARD PA_7 //PWM
#define ROTOR_MOTOR_REVERSE PA_6 //PWM

#define TAPE_SENSOR_POT PA5 //ANALOG
#define DISPLAY_BUTTON PA4 //DIGITAL

#define LEFT_TAPE_SENSOR PA3 //ANALOG
#define RIGHT_TAPE_SENSOR PA2 //ANALOG

#define LEFT_MOTOR_FORWARD PA_1 //PWM
#define RIGHT_MOTOR_FORWARD PA_0 //PWM

#define REAR_SERVO PA_8 //PWM

#define LEFT_MOTOR_REVERSE PB_8 //PWM
#define RIGHT_MOTOR_REVERSE PB_9 //PWM

#define RIGHT 0
#define LEFT 1

#define PWM_FREQUENCY 1000

#define STOP 0
#define FULL_SPEED 512
#define ROTOR_SPEED 512 * 0.75

#define FORWARD 1
#define REVERSE 0

#define UP_ANGLE 90
#define DOWN_ANGLE 38

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void moveServo(int);
void countDown();
void displayInfo();
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
void turnRotor(int, int);

bool rightReflectance;
bool leftReflectance;
unsigned int tapeSensorThreahold;

int lastSideOnTape = RIGHT;

unsigned int correctingSpeed;
unsigned int driveSpeed;

void setup() {
  // initialize LED digital pin as an output and set on
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(RIGHT_MOTOR_FORWARD, OUTPUT);
  pinMode(RIGHT_MOTOR_REVERSE, OUTPUT);

  pinMode(LEFT_MOTOR_FORWARD, OUTPUT);
  pinMode(LEFT_MOTOR_REVERSE, OUTPUT);

  pinMode(DISPLAY_BUTTON, INPUT_PULLUP);

  moveServo(UP_ANGLE);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  refreshDisplay();

  delay(1000);
  //countDown();
  moveServo(DOWN_ANGLE);
  delay(100);
  }

void loop() {
  driveSpeed = map(analogRead(DRIVE_SPEED_POT), 0, 1023, 0, FULL_SPEED);
  readCorrectionSpeed();
  readTapeSensors();

  if (digitalRead(DISPLAY_BUTTON)){
    correctDirection();
    turnRotor(ROTOR_SPEED, FORWARD);

  }
  else{
    driveStraight(STOP);
    displayInfo();
  }
  
}

void moveServo(int angle){
  int angleInMicroseconds = map(angle, 0, 180, 500, 2500);
  pwm_start(REAR_SERVO, 50, angleInMicroseconds, TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
}

void countDown(){
  refreshDisplay();
  printMessage("3", true);
  display.display();
  delay(1000);
  printMessage("2", true);
  display.display();
  delay(1000);
  printMessage("1", true);
  display.display();
  delay(1000);
  printMessage("Starting", true);
  display.display();
}

void displayInfo(){
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

void readCorrectionSpeed(){
  correctingSpeed = map(analogRead(CORRECTING_SPEED_POT), 0, 1023, 0, FULL_SPEED);
}

void correctDirection(){
  if(rightReflectance && leftReflectance){
    driveStraight(driveSpeed);
  }
  else if (rightReflectance){
    turn(RIGHT);
  }
  else if (leftReflectance){
    turn(LEFT);
  }
  else{
    turnWithReverse(lastSideOnTape);    
  }
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
  tapeSensorThreahold = analogRead(TAPE_SENSOR_POT);

  rightReflectance = analogRead(RIGHT_TAPE_SENSOR) > tapeSensorThreahold;
  leftReflectance = analogRead(LEFT_TAPE_SENSOR) > tapeSensorThreahold;

  if (rightReflectance && !leftReflectance){
    lastSideOnTape = RIGHT;
  }
  else if (leftReflectance && !rightReflectance){
    lastSideOnTape = LEFT;
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
    driveMotor(RIGHT, correctingSpeed, REVERSE);
  }
  else if (side == LEFT) {
    driveMotor(RIGHT, correctingSpeed, FORWARD);
    driveMotor(LEFT, correctingSpeed, REVERSE);
  }
}

void driveStraight(int speed){
  driveMotor(RIGHT, speed, FORWARD);
  driveMotor(LEFT, speed, FORWARD);
}

void driveMotor(int motorSide, int speed, int direction){
  if (motorSide == RIGHT) {
    if (direction == FORWARD) {
      pwm_start(RIGHT_MOTOR_REVERSE, PWM_FREQUENCY, STOP, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(RIGHT_MOTOR_FORWARD, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
    else if (direction == REVERSE){
      pwm_start(RIGHT_MOTOR_FORWARD, PWM_FREQUENCY, STOP, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(RIGHT_MOTOR_REVERSE, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
  }
  else if (motorSide == LEFT) {
    if (direction == FORWARD) {
      pwm_start(LEFT_MOTOR_REVERSE, PWM_FREQUENCY, STOP, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(LEFT_MOTOR_FORWARD, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
    else if (direction == REVERSE){
      pwm_start(LEFT_MOTOR_FORWARD, PWM_FREQUENCY, STOP, RESOLUTION_9B_COMPARE_FORMAT);
      pwm_start(LEFT_MOTOR_REVERSE, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
    }
  }
}

void turnRotor(int speed, int direction){
  if (direction == FORWARD){
    pwm_start(ROTOR_MOTOR_REVERSE, PWM_FREQUENCY, STOP, RESOLUTION_9B_COMPARE_FORMAT);
    pwm_start(ROTOR_MOTOR_FORWARD, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
  }
  else if (direction == REVERSE){
    pwm_start(ROTOR_MOTOR_FORWARD, PWM_FREQUENCY, STOP, RESOLUTION_9B_COMPARE_FORMAT);
    pwm_start(ROTOR_MOTOR_FORWARD, PWM_FREQUENCY, speed, RESOLUTION_9B_COMPARE_FORMAT);
  }
}
