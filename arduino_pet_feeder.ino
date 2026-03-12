#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ===== CONFIG =====
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo servo;
const int servoPin = 7;

const int trigPin = 8;
const int echoPin = 9;
const long bowlFullDist = 25; // cm, distance for "full bowl"

// DISPENSE INTERVAL
// 2 hours = 2*60*60*1000 ms
// For testing: set to 10000 = 10 seconds
const unsigned long dispenseInterval = 30000UL; 


unsigned long lastDispense = 0;

// ===== SETUP =====
void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Pet Feeder Ready");

  servo.attach(servoPin);
  servo.write(0); // closed

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(9600);
}

// ===== FUNCTIONS =====
long getDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if(duration == 0) return 999; // no reading
  long dist = duration * 0.034 / 2;
  return dist;
}

void dispenseFood() {
  Serial.println("Dispensing food...");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Dispensing...");
  servo.write(150);   // open
  delay(2000);       // adjust for food amount
  servo.write(0);    // close
  lastDispense = millis();
  lcd.setCursor(0,1);
  lcd.print("Done");
  delay(1000);
}

// ===== LOOP =====
void loop() {
  long distance = getDistanceCM();

  // DISPLAY
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Bowl: ");
  lcd.print(distance);
  lcd.print("cm");

  lcd.setCursor(0,1);
  lcd.print("Next in: ");
  unsigned long now = millis();
  unsigned long remain = 0;
  if(now >= lastDispense) {
    unsigned long passed = now - lastDispense;
    if(passed < dispenseInterval) {
      remain = (dispenseInterval - passed) / 1000; // seconds
      lcd.print(remain);
      lcd.print("s");
    } else {
      lcd.print("0s");
    }
  }

  // AUTO DISPENSE if bowl not full
  if(distance > bowlFullDist && (millis() - lastDispense >= dispenseInterval)) {
    dispenseFood();
  }

  delay(500); // avoid flicker
}
