#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <algorithm>    // std::sort
#include <vector>       // std::vector

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible

#define ROTOR_SPEED_POT PB1 //ANALOG
#define CORRECTING_SPEED_POT PB0 //ANALOG

#define COMBINE_MOTOR_FORWARD PA_7 //PWM
#define COMBINE_MOTOR_REVERSE PA_6 //PWM

#define TAPE_SENSOR_POT PA5 //ANALOG
#define DISPLAY_BUTTON PA4 //DIGITAL

#define LEFT_TAPE_SENSOR PA3 //ANALOG
#define RIGHT_TAPE_SENSOR PA2 //ANALOG

#define LEFT_MOTOR_FORWARD PA_1 //PWM
#define RIGHT_MOTOR_FORWARD PA_0 //PWM

#define REAR_SERVO PA_8 //PWM  //TODO: *Check the underscore in the pin name*

#define LEFT_MOTOR_REVERSE PB_8 //PWM
#define RIGHT_MOTOR_REVERSE PB_9 //PWM

#define RIGHT 0
#define LEFT 1

#define RIGHT 0
#define LEFT 1

#define PWM_FREQUENCY 1000

#define STOP 0
#define FULL_SPEED 512
#define REVERSE_SPEED 512

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
void callibrate();
void correctDirectionPD();
void test();
void readSpeeds();
void blink();

bool rightReflectance;
bool leftReflectance;
unsigned int tapeSensorThreahold;
int surface;
int tape;
double lasterror;
double kd;
double kp;
int lastSideOnTape = RIGHT;
bool startup;

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
  
  pinMode(COMBINE_MOTOR_FORWARD, OUTPUT);

  pinMode(DISPLAY_BUTTON, INPUT_PULLUP);

  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //display.display();
  delay(2000);
  // display.clearDisplay();
  // display.setTextSize(2);
  // display.setTextColor(SSD1306_WHITE);
  // display.setCursor(0,0);
  startup = true;

  }

void loop() {
  
  

  if(startup){
    blink();
    callibrate();
    blink();
    test();
    startup = false;
    printf("DONE CALLIBRATION\n TAPE IS: %d\n SURFACE IS: %d\n", tape, surface);
  }
  // //correctDirectionPD();  
  readSpeeds();
  readTapeSensors();
  //  correctDirectionPD();
  printf("speed %d\nrighttape %d\nlefttape %d\n", correctingSpeed*100/512, rightReflectance, leftReflectance);
  if (digitalRead(DISPLAY_BUTTON))
  {
    correctDirection();
    pwm_start(COMBINE_MOTOR_FORWARD, PWM_FREQUENCY, driveSpeed/5, RESOLUTION_9B_COMPARE_FORMAT);
  
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

void test(){
  
  readSpeeds();
  driveMotor(RIGHT,correctingSpeed,FORWARD);
  digitalWrite(LED_BUILTIN,HIGH);
  delay(2000);
  driveMotor(RIGHT,driveSpeed,REVERSE);
  digitalWrite(LED_BUILTIN,0);
  delay(2000);
  driveMotor(LEFT,correctingSpeed,FORWARD);
  digitalWrite(LED_BUILTIN,HIGH);
  delay(2000);
  driveMotor(LEFT,driveSpeed,REVERSE);
  digitalWrite(LED_BUILTIN,0);
  delay(2000);
  blink();
  delay(5000);

}
void readSpeeds(){
  readCorrectionSpeed();
  driveSpeed = map(analogRead(ROTOR_SPEED_POT), 0, 1023, 0, FULL_SPEED);
}

void readCorrectionSpeed(){
  correctingSpeed = map(analogRead(CORRECTING_SPEED_POT), 0, 1023, 0, FULL_SPEED);
}

// MOVE ROBOT SIDE TO SIDE OVER TAPE AT START TO GET GOOD VALUES
//
// creates a range of values that varies between surface and tape values due to addition of sensors.
// Hope low 100 are all on surface and high 100 are all on tape. 
// Creates tape sensor threshold which is the middle values between tape and surface. 
// Robot will follow tape with one sensor on and one sensor off
void callibrate(){
  int size = 5000;
  int sensorArray[size] = {};
  int temp = 0;
  for(int i=0; i<size; i++){
    if(i == size / 2){
      blink();
      blink();
      delay(5000);
    }
    sensorArray[i] = analogRead(RIGHT_TAPE_SENSOR)+analogRead(LEFT_TAPE_SENSOR);
   // printf("Sensor val: %d", sensorArray[i]);
  }
  for(int i=0;i<size;i++)
	{		
		for(int j=i+1;j<size;j++)
		{
			if(sensorArray[i]>sensorArray[j])
			{
				temp  =sensorArray[i];
				sensorArray[i]=sensorArray[j];
				sensorArray[j]=temp;
			}
		}
    //printf("sorting....");
	}
  //printf("sorted");
  //delay(1000);
  int surfacesum = 0;
  int tapesum = 0;
  for(int j = 0; j<100; j++){
     //printf("sensorstart: %d\nsensorend: %d\n", sensorArray[j],sensorArray[j+size-101]);
     surfacesum += sensorArray[j];
     tapesum += sensorArray[j+size-101];
  }

  tape = tapesum / 100;
  surface = surfacesum / 100; 
  //normalize threshhold for one sensor
  tapeSensorThreahold = (tape - surface) /4 + surface/2;
  //printf("tape: %d\n Surface: %d\n", tape, surface);
  
}

void blink(){
  for(int i = 0; i<5; i++){
    digitalWrite(LED_BUILTIN,HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN,0);
    delay(100);

  }
}

void correctDirectionPD(){
    double p = analogRead(TAPE_SENSOR_POT)/100.0;
    kd = 0;
    int error = (1600-72)/2 - analogRead(RIGHT_TAPE_SENSOR)- analogRead(LEFT_TAPE_SENSOR);
    int adjustment = error*p + (lasterror-error)*kd;

    lasterror = error;

    //printf("ERROR: %d\n ADJUSTMENT %d\nKP*1000 %x",error,adjustment,p*1000);
    if(correctingSpeed + adjustment < 0){
      driveMotor(RIGHT,constrain(-1*(correctingSpeed + adjustment), 0, correctingSpeed) , REVERSE);
      driveMotor(LEFT, constrain(correctingSpeed - adjustment,0, correctingSpeed), FORWARD);
    }else if(correctingSpeed - adjustment < 0){
      driveMotor(RIGHT,constrain(correctingSpeed + adjustment, 0, correctingSpeed) , FORWARD);
      driveMotor(LEFT, constrain(-1*(correctingSpeed - adjustment),0, correctingSpeed), REVERSE);
    }else{
      driveMotor(RIGHT,constrain(correctingSpeed + adjustment, 0, correctingSpeed) , FORWARD);
      driveMotor(LEFT, constrain(correctingSpeed - adjustment,0, correctingSpeed), FORWARD);
    }
      
}




void correctDirection(){
  if(rightReflectance && leftReflectance){
    digitalWrite(LED_BUILTIN,HIGH);
    driveStraight(driveSpeed);
    //printMessage("Straight", true);
  }
  else if (rightReflectance){
    digitalWrite(LED_BUILTIN,0);
    turn(RIGHT);
    //printMessage("Turn Right", true);
  }
  else if (leftReflectance){
    digitalWrite(LED_BUILTIN,0);
    turn(LEFT);
    //printMessage("Turn Left", true);
  }
  else{
    turnWithReverse(lastSideOnTape);
    digitalWrite(LED_BUILTIN,0);
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
  //tapeSensorThreahold = analogRead(TAPE_SENSOR_POT);

  rightReflectance = analogRead(RIGHT_TAPE_SENSOR) > tapeSensorThreahold;
  leftReflectance = analogRead(LEFT_TAPE_SENSOR) > tapeSensorThreahold;
 // printf("Threshhold %d\n Left %d\n right %d", tapeSensorThreahold,leftReflectance,rightReflectance);

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
    driveMotor(RIGHT, correctingSpeed*REVERSE_SPEED/FULL_SPEED, REVERSE);
  }
  else if (side == LEFT) {
    driveMotor(RIGHT, correctingSpeed, FORWARD);
    driveMotor(LEFT, correctingSpeed*REVERSE_SPEED/FULL_SPEED, REVERSE);
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
