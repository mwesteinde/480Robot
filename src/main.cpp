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

#define maxspeed 511 //max speed of motor
#define maxspeedl 511
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
int fifthspeed = -(maxspeed - 100);
int halfspeedl = maxspeedl - 200;
int thirdspeedl = maxspeedl/3;
int fourthspeedl = 0;
int fifthspeedl = -(maxspeedl - 100);
//int i = 300;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void runmotors(int speedl, int speedr);
void showvalues();
void readpots();
void hitchcontrol(int angle);
void partymode();
void ballrelease();
void shoot();
void movebot(int timel, int timer);


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
    display.clearDisplay();
    display.println("Partayyyy");
    display.display();
    hitchcontrol(180);
    partymode();
  }
  hitchcontrol(180);
  if(digitalRead(startpin)) {
    display.clearDisplay();
    display.println("Hitch Test");
    display.display();
    while (digitalRead(displayswitch)) {
      hitchcontrol(180);
      delay(20);
    } 
    hitchcontrol(130);
    delay(1000);
    hitchcontrol(180);
    display.clearDisplay();
    display.println("Ready2Rumble");
    display.display();
    while (digitalRead(displayswitch)) {
      hitchcontrol(180);
      delay(50);
    } 
    display.clearDisplay();
    display.println("Run Forrest!");
    display.display();
  }
  hitchcontrol(130);
  delay(500);
  attachInterrupt(digitalPinToInterrupt(displayswitch), showvalues, FALLING);
  pwm_start(rotor, 1024, rotorspeed, RESOLUTION_9B_COMPARE_FORMAT);
  pwm_start(motorlf, 512, 512, RESOLUTION_9B_COMPARE_FORMAT);
  pwm_start(motorrf, 512, 512, RESOLUTION_9B_COMPARE_FORMAT);
  pinMode(correctingspeed, INPUT_ANALOG);
}

/*void loop() {
  int speed = map(analogRead(correctingspeed), 0, 1024, 0, 512);
  pwm_start(motorlf, 512, speed, RESOLUTION_9B_COMPARE_FORMAT);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(speed);
  display.display();
  delay(1000);
}*/


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
    runmotors(maxspeedl, maxspeed);
  }

  if (tapeonl && !tapeonr) {
    lastonl = true;
    if (lastr) {
      runmotors(thirdspeedl, maxspeed);
    } else {
      runmotors(halfspeedl, maxspeed);
    }
    
  }

  if (tapeonr && !tapeonl) {
    lastonl = false;
    if (lastl) {
      runmotors(maxspeedl, thirdspeed);
    } else {
      runmotors(maxspeedl, halfspeed);
    }
  }

  if (!tapeonr && !tapeonl) {
    if (lastonl) {
      if (lastl) {
        runmotors(fourthspeedl, maxspeed);
      } else {
        runmotors(fifthspeedl, maxspeed);
      }
    } else {
      if (lastr) {
        runmotors(maxspeedl, fourthspeed);
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
    ballrelease();
    delay(500);
    ballrelease();
    delay(3000);
    shoot();
    delay(500);
    ballrelease();
    delay(3000);
    shoot();
    delay(500);
    movebot(600, 350);
    ballrelease();
    delay(3000);
    shoot();
    delay(500);
    ballrelease();
    delay(3000);
    shoot();
    delay(500);
    movebot(-250, 250);
    ballrelease();
    delay(3000);
    shoot();
    delay(500);
    ballrelease();
    delay(3000);
    shoot();
    delay(500);
  }


void shoot() {
  pwm_start(rotor, 512, 512, RESOLUTION_9B_COMPARE_FORMAT);
  delay(440);
  pwm_start(rotor, 512, STOP, RESOLUTION_9B_COMPARE_FORMAT);

}

void movebot(int timel, int timer) {
  if (timel > 0) {
    pwm_start(motorlb, 512, 400, RESOLUTION_9B_COMPARE_FORMAT);
    delay(timel);
    pwm_start(motorlb, 512, STOP, RESOLUTION_9B_COMPARE_FORMAT);
  } else {
    pwm_start(motorlf, 512, 400, RESOLUTION_9B_COMPARE_FORMAT);
    delay(-timel);
    pwm_start(motorlf, 512, STOP, RESOLUTION_9B_COMPARE_FORMAT);
  }
  if (timer > 0) {
   pwm_start(motorrb, 512, 400, RESOLUTION_9B_COMPARE_FORMAT);
   delay(timer);
   pwm_start(motorrb, 512, STOP, RESOLUTION_9B_COMPARE_FORMAT);
  } else {
    pwm_start(motorrf, 512, 400, RESOLUTION_9B_COMPARE_FORMAT);
   delay(-timer);
   pwm_start(motorrf, 512, STOP, RESOLUTION_9B_COMPARE_FORMAT);
  }


}

void ballrelease() {
  pwm_start(ballservo, 50, 1600, TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
  delay(99);
  pwm_start(ballservo, 50, 2500, TimerCompareFormat_t::MICROSEC_COMPARE_FORMAT);
}

