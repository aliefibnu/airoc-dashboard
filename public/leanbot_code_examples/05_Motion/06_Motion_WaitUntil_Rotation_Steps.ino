#include <Leanbot.h>
 
void setup() {
  Leanbot.begin();                            // initialize Leanbot
}
 
void loop() {
  int speed = 1000;                           // Speed of movement
 
  // the number of steps for Leanbot to turn an angle of approximately 90 degrees
  // need to change with each Leanbot, running speed, working surface
  int rotation90 = 1675;
 
  LbMotion.runLR(+speed, -speed);             // let Leanbot turn right
  LbMotion.setZeroOrigin();
  // Rotate 360 degrees, split into approximately 90 degree angles:
  for (int rotation = 0; rotation <= (rotation90*4); rotation += rotation90) {
    LbMotion.waitUntilRotation(rotation);
    Leanbot.tone(440*4, 25);                  // Beep every time it rotates 90 degrees
  }
  LbMotion.stopAndWait();                     // let Leanbot stop slowly
 
  LbDelay(3000);                              // delay 3s
 
 
  LbMotion.runLR(-speed, +speed);             // let Leanbot turn left
  LbMotion.setZeroOrigin();
  // Rotate 360 degrees, split into approximately 90 degree angles:
  for (int rotation = 0; rotation <= (rotation90*4); rotation += rotation90) {
    LbMotion.waitUntilRotation(-rotation);
    Leanbot.tone(440*4, 25);
  }
  LbMotion.stopAndWait();
 
  LbDelay(3000);
}
