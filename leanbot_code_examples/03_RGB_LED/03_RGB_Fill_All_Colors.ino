#include <Leanbot.h>
 
 
void setup() {
  Leanbot.begin();                    // initialize Leanbot
}
 
 
void loop() {
  LbRGB.fillColor(CRGB::Green);       // green setting for all lights
  LbRGB.show();                       // LED shows up
  delay(1000);                        // light time
 
  LbRGB.fillColor(CRGB::Blue);        // blue setting for all lights
  LbRGB.show();                       // LED shows up
  delay(1000);                        // light time
}
