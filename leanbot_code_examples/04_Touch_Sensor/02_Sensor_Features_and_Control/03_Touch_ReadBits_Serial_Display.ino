#include <Leanbot.h>
 
int pinLed = 13;                 // declare pins connected to LED
 
void setup() {
  // put your setup code here, to run once:
  Leanbot.begin();               // initialize Leanbot
  pinMode(pinLed, OUTPUT);       // Configure the LED driver pin to be OUTPUT
}
 
void printButtons(int bits) {
  for (int i = 3; i >= 0; i--) {
    if (bitRead(bits, i)) {
      Serial.print(" #");
    } else {
      Serial.print(" .");
    }
  }
  Serial.println();
}
 
void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
 
  int touchBits = LbTouch.readBits();
  printButtons(touchBits);
 
  if (touchBits == 0b1111) {
    digitalWrite(pinLed, HIGH);  // If all 4 buttons are touched then the light turns on
  } else {
    digitalWrite(pinLed, LOW);   // If not, turn off the light
  }
}
