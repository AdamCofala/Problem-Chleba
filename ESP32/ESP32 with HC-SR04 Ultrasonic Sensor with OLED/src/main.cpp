#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int trigPin = 5;
const int echoPin = 18;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

void setup_dist(){
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
}

void setup_oled(){

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 25);
  display.println("Starting...");
  display.display();

}

float get_distance(){

  long duration;
  float distanceInch;

  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  return duration * SOUND_SPEED/2;
}

void show_on_oled(float distance) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  char buf[32];
  snprintf(buf, sizeof(buf), "Distance: \n\n %.1f cm", distance);

  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);

  int x = (128 - w) / 2; // center horizontally
  int y = (64 - h) / 2;  // center vertically

  display.setCursor(x, y);
  display.print(buf);
  display.display();
}


void setup() {
  Serial.begin(115200);
  setup_dist();
  setup_oled();
  delay(2500);
}

void loop() {
  
  Serial.print("Distance (cm): ");
  Serial.println(get_distance());
  show_on_oled(get_distance());
  delay(200);
}
