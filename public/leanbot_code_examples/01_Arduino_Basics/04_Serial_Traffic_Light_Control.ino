#include <Leanbot.h>
 
void setup() {
  Leanbot.begin();    // initialize Leanbot
  Serial.begin(9600);
}
 
void loop() {
  if (Serial.available() > 0) {
    int userInput = Serial.read() - '0'; // convert ASCII to integer
    switch (userInput) {
      case 1:
        // Red light on
        Leanbot.tone(1500,30);
        LbRGB[ledO] = CRGB::Red;           
        LbRGB.show();                      
        Serial.println("Red light is on");
        break;
      case 2:
        // Yellow light on
        Leanbot.tone(1500,30);
        LbRGB[ledO] = CRGB::Yellow;          
        LbRGB.show();                     
        Serial.println("Yellow light is on");
        break;
      case 3:
        Leanbot.tone(1500,30);
        LbDelay(1000);   
        Leanbot.tone(1500,30);
         LbDelay(1000);   
        Leanbot.tone(1500,30);
        LbRGB[ledO] = CRGB::Green;          
        LbRGB.show();                     
        Serial.println("Green light is on");
        break;
      default:
        Serial.println("Invalid input");
    }
  }
}
