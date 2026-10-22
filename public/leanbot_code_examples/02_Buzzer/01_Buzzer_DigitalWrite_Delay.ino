#include <Leanbot.h>
 
void setup() {
  Leanbot.begin();                    // initialize Leanbot
  pinMode(LBPIN_BUZZER, OUTPUT);
}
 
void loop() {
  digitalWrite(LBPIN_BUZZER, HIGH);
  delay(10);
  digitalWrite(LBPIN_BUZZER, LOW);
  delay(10);
}
