#include <Leanbot.h>
 
 
void setup() {
  Leanbot.begin();                    // initialize Leanbot
}
 
 
void loop() {
  // Turn on the lights one by one
  for (int i = 0; i < 7; i++) {
    LbRGB[i] = CRGB::Blue;             // color settings
    LbRGB.show();                      // LED shows up
    LbDelay(400);                      // time delay between lights
  }
 
  // Turn off the lights one by one
  for (int i = 0; i < 7; i++) {
    LbRGB[i] = CRGB::Black;            // color settings
    LbRGB.show();                      // LED shows up
    LbDelay(400);                      // time delay between lights
  }
}
