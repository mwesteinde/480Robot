#include <Wire.h>
#include <Adafruit_SSD1306.h>
//#include <Servo.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible

#define tapel PA3 //right tape sensor
#define taper PA2 //left tape sensor

#define motorlb PB_0 //left motor forward
#define motorlf PB_1 //left motor reverse
#define motorrb PA_6 //right motor forward
#define motorrf PA_7 //right motor reverse

#define rotor PB_9

#define displayswitch PB4

#define servopin PA_8

#define startpin PB5

#define maxspeed 1024 //max speed of motor
#define threshold 400 //tape on/off threshold
#define rotorspeed 512

bool tapeonl;
bool tapeonr;
bool lastl;
bool lastr;

int speedl;
int speedr;

//bool speedsl[8];
//bool speedsr[8];

bool lastonl;

int halfspeed = maxspeed/2;
int thirdspeed = -maxspeed;
int fourthspeed = -maxspeed;
//int i = 300;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void runmotors(int speedl, int speedr);
void showvalues();
void hitchcontrol(int angle);

void setup() {
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // Displays Adafruit logo by default. call clearDisplay immediately if you don't want this.
  display.display();
  delay(250);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  pinMode(displayswitch, INPUT_PULLUP);
  pinMode(startpin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(displayswitch), showvalues, FALLING);
  hitchcontrol(180);
  while(digitalRead(startpin)) {
  }
  delay(1000);
  hitchcontrol(130);
  delay(500);
  pwm_start(rotor, 1024, rotorspeed, RESOLUTION_10B_COMPARE_FORMAT);
}

void loop() {
  if (analogRead(tapel) > threshold) {
    tapeonl = true;
    lastl = true;
  } else {
    if (tapeonl == false) {
      lastl = false;
    }
    tapeonl = false;
  }
  if (analogRead(taper) > threshold) {
    tapeonr = true;
    lastr = true;
  } else {
    if (tapeonr == false) {
      lastr = false;
    }
    tapeonr = false;

  }

  if (tapeonl && tapeonr) {
    runmotors(maxspeed, maxspeed);
  }

  if (tapeonl && !tapeonr) {
    lastonl = true;
    runmotors(halfspeed, maxspeed);
  }

  if (tapeonr && !tapeonl) {
    lastonl = false;
    runmotors(maxspeed, halfspeed);
  }

  if (!tapeonr && !tapeonl) {
    if (lastonl) {
      runmotors(thirdspeed, maxspeed);
    } else {
      runmotors(maxspeed, thirdspeed);
    }
  }
  if (digitalRead(startpin)) {
    delay(10);
  } else {
    delay(10);
  }

  


}

void runmotors(int speedl, int speedr) {
  if (speedl >= 0) {
    pwm_start(motorlb, 1024, 0, RESOLUTION_10B_COMPARE_FORMAT);
    pwm_start(motorlf, 1024, speedl, RESOLUTION_10B_COMPARE_FORMAT);
  } else {
    pwm_start(motorlf, 1024, 0, RESOLUTION_10B_COMPARE_FORMAT);
    pwm_start(motorlb, 1024, -speedl, RESOLUTION_10B_COMPARE_FORMAT);
  }
  if (speedr >= 0) {
    pwm_start(motorrb, 1024, 0, RESOLUTION_10B_COMPARE_FORMAT);
    pwm_start(motorrf, 1024, speedr, RESOLUTION_10B_COMPARE_FORMAT);
  } else {
    pwm_start(motorrf, 1024, 0, RESOLUTION_10B_COMPARE_FORMAT);
    pwm_start(motorrb, 1024, -speedr, RESOLUTION_10B_COMPARE_FORMAT);
  }
  
  

}

void hitchcontrol(int angle) {
  int pwmsignal = map(angle, 0, 180, 500, 2500);
  pwm_start(servopin, 50, pwmsignal, TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
}

void showvalues() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(analogRead(tapel));
  display.println(analogRead(taper));
  display.display();
}