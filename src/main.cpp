#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible

#define tapel PA3 //right tape sensor
#define taper PA2 //left tape sensor

#define motorlb PB_1 //left motor forward
#define motorlf PB_0 //left motor reverse
#define motorrb PA_6 //right motor forward
#define motorrf PA_7 //right motor reverse

#define rotor PB_9

#define displayswitch PB4

#define servopin PA_8
#define ballservo PA_1

#define startpin PB5

#define correctingspeed PA0
#define turnspeed PA1

#define maxspeed 512 //max speed of motor
#define max2 400
#define threshold 400 //tape on/off threshold
#define rotorspeed 350
#define pwmfrequency 512
#define partyspeed 1023
#define STOP 0

bool tapeonl;
bool tapeonr;
bool lastl;
bool lastr;

int speedl;
int speedr;
int i = 100;

//bool speedsl[8];
//bool speedsr[8];

bool lastonl;

int halfspeed = maxspeed - 150;
int thirdspeed = maxspeed/2;
int fourthspeed = 0;
int fifthspeed = -maxspeed - 150;
//int i = 300;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void runmotors(int speedl, int speedr);
void showvalues();
void readpots();
void hitchcontrol(int angle);
void partymode();
void ballrelease();
void shoot();


void setup() {
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
   //Displays Adafruit logo by default. call clearDisplay immediately if you don't want this.
  display.display();
  delay(250);
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  pinMode(displayswitch, INPUT_PULLUP);
  pinMode(startpin, INPUT_PULLUP);
  pinMode(correctingspeed, INPUT_PULLDOWN);
  pinMode(turnspeed, INPUT_ANALOG);

  if(digitalRead(correctingspeed)) {
    display.clearDisplay();
    display.println("PartyMode Ready");
    display.display();
    while(digitalRead(displayswitch)) {
    }
    pinMode(correctingspeed, INPUT_ANALOG);
    partymode();
  }
  hitchcontrol(180);
  if(digitalRead(startpin)) {
    display.clearDisplay();
    display.println("Ready2Rumble");
    display.display();
    while (digitalRead(displayswitch)) {
      hitchcontrol(180);
      delay(20);
    } 
    display.clearDisplay();
    display.println("Run Forrest!");
    display.display();
  }
  attachInterrupt(digitalPinToInterrupt(displayswitch), showvalues, FALLING);
  //delay(1000);
  hitchcontrol(130);
  delay(500);
  pwm_start(rotor, 1024, rotorspeed, RESOLUTION_9B_COMPARE_FORMAT);
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
    if (lastr) {
      runmotors(thirdspeed, maxspeed);
    } else {
      runmotors(halfspeed, maxspeed);
    }
    
  }

  if (tapeonr && !tapeonl) {
    lastonl = false;
    if (lastl) {
      runmotors(maxspeed, thirdspeed);
    } else {
      runmotors(maxspeed, halfspeed);
    }
  }

  if (!tapeonr && !tapeonl) {
    if (lastonl) {
      if (lastl) {
        runmotors(fourthspeed, maxspeed);
      } else {
        runmotors(fifthspeed, maxspeed);
      }
    } else {
      if (lastr) {
        runmotors(maxspeed, fourthspeed);
      } else {
        runmotors(maxspeed, fifthspeed);
      }
    }
  }
  delay(10);
}

void runmotors(int speedl, int speedr) {
  if (speedl >= 0) {
    pwm_start(motorlb, pwmfrequency, 0, RESOLUTION_9B_COMPARE_FORMAT);
    pwm_start(motorlf, pwmfrequency, speedl, RESOLUTION_9B_COMPARE_FORMAT);
  } else {
    pwm_start(motorlf, pwmfrequency, 0, RESOLUTION_9B_COMPARE_FORMAT);
    pwm_start(motorlb, pwmfrequency, -speedl, RESOLUTION_9B_COMPARE_FORMAT);
  }
  if (speedr >= 0) {
    pwm_start(motorrb, pwmfrequency, 0, RESOLUTION_9B_COMPARE_FORMAT);
    pwm_start(motorrf, pwmfrequency, speedr, RESOLUTION_9B_COMPARE_FORMAT);
  } else {
    pwm_start(motorrf, pwmfrequency, 0, RESOLUTION_9B_COMPARE_FORMAT);
    pwm_start(motorrb, pwmfrequency, -speedr, RESOLUTION_9B_COMPARE_FORMAT);
  }
}

void hitchcontrol(int angle) {
  int pwmsignal = map(angle, 0, 180, 500, 2500);
  pwm_start(servopin, 50, pwmsignal, TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
}

void readpots(){
  thirdspeed = map(analogRead(correctingspeed), 0, 1024, 0, 511);
  fifthspeed = -map(analogRead(turnspeed), 0, 1024, 0, 511);
}

void showvalues() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("left: ");
  display.println(analogRead(tapel));
  display.print("right: ");
  display.println(analogRead(taper));

  display.display();
}

void partymode() {
  while(true) {
    ballrelease();
    delay(500);
    pwm_start(rotor, 512, 512, RESOLUTION_9B_COMPARE_FORMAT);
    delay(300);
    pwm_start(rotor, 512, STOP, RESOLUTION_9B_COMPARE_FORMAT);
    delay(500);
  }
}

void shoot() {

}

void ballrelease() {
  //int pwmsignal = map(660, 0, 1024, 500, 2500);
  pwm_start(ballservo, 50, 1600, TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
  /*display.clearDisplay();
  display.setCursor(0,0);
  display.println(analogRead(correctingspeed));
  display.display();*/
  delay(98);
  pwm_start(ballservo, 50, 2500, TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
  delay(3000);
}

