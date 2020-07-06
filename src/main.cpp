#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET 	-1 // This display does not have a reset pin accessible

#define tapel PB0
#define taper PB1
#define motorlf PA_2
#define motorlb PA_3
#define motorrf PA_0
#define motorrb PA_1
#define threshold 875
#define maxspeed 512
#define switch PB12

bool tapeonl;
bool tapeonr;

int speedl;
int speedr;

bool speedsl[8];
bool speedsr[8];

bool lastonl;

int halfspeed = maxspeed/2;
int thirdspeed = 0;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
void runmotors(int speedl, int speedr);
void showvalues();

void setup() {
   display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // Displays Adafruit logo by default. call clearDisplay immediately if you don't want this.
  display.display();
  delay(2000);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.clearDisplay();
  display.setCursor(0,0);
  display.println(analogRead(tapel));
  display.println(analogRead(taper));
  display.display();
  delay(2000);
   display.clearDisplay();
  display.setCursor(0,0);
  display.println(analogRead(tapel));
  display.println(analogRead(taper));
  display.display();
  delay(2000);
  pinMode(switch, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switch), showvalues, FALLING);


}

void loop() {
  if (analogRead(tapel) > threshold) {
    tapeonl = true;
  } else {
    tapeonl = false;
  }
  if (analogRead(taper) > threshold) {
    tapeonr = true;
  } else {
    tapeonr = false;
  }

  if (tapeonl && tapeonr) {
    runmotors(maxspeed, maxspeed);
  }

  if (tapeonl && !tapeonr) {
    lastonl = true;
    runmotors(maxspeed, halfspeed);
  }

  if (tapeonr && !tapeonl) {
    lastonl = false;
    runmotors(halfspeed, maxspeed);
  }

  if (!tapeonr && !tapeonl) {
    if (lastonl) {
      runmotors(maxspeed, thirdspeed);
    } else {
      runmotors(thirdspeed, maxspeed);
    }
  }
}

void runmotors(int speedl, int speedr) {
  pwm_start(motorrf, 1024, speedr, RESOLUTION_10B_COMPARE_FORMAT);
  pwm_start(motorlf, 1024, speedl, RESOLUTION_10B_COMPARE_FORMAT);


}

void showvalues() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(analogRead(tapel));
  display.println(analogRead(taper));
  display.display();
}