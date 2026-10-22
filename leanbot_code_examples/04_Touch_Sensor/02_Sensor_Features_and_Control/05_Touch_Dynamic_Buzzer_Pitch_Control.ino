#include <Leanbot.h>
 
int f;
int d;
 
void setup() {
  Leanbot.begin();               // initialize Leanbot
}
 
void loop() {
  for (f=635; f<=912; f++) {
    Leanbot.tone(f);
    delay(d);
 
    if (LbTouch.read(TB1A) == HIGH)
      f = 912;
 
    if (LbTouch.read(TB1B) == HIGH)
      d = 0;
    else
      d = 7;
  }
 
  for (f=911; f>=634; f--) {
    Leanbot.tone(f);
    delay(d);
 
    if (LbTouch.read(TB1A) == HIGH)
      f = 634;
 
    if (LbTouch.read(TB1B) == HIGH)
      d = 0;
    else
      d = 10;
  }
 
  while (LbTouch.read(TB1A) == HIGH) {
    Leanbot.noTone();
    Leanbot.tone(250);
    delay(5);
    Leanbot.noTone();
    delay(5);
  }
}
