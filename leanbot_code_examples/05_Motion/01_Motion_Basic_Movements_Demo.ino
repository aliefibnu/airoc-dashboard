#include <Leanbot.h>
 
void setup() {
  Leanbot.begin();                 // initialize Leanbot
}
 
void loop() {
  example1();
  example2();
  example3();
  example4();
  example5();
}
 
void example1() {
  LbMotion.runLR(+400, +400);      // let Leanbot go straight
  LbMotion.waitDistanceMm(+150);   // wait for Leanbot to reach 150mm = 15cm
  LbMotion.runLR(-400, -400);      // give Leanbot back
  LbMotion.waitDistanceMm(-150);   // wait for Leanbot to reach 150mm = 15cm
}
 
void example2() {
  LbMotion.runLR(+400, +500);      // let Leanbot move to the left
  LbMotion.waitDistanceMm(+150);   // wait for Leanbot to reach 150mm = 15cm
  LbMotion.runLR(-400, -500);
  LbMotion.waitDistanceMm(-150);
}
 
void example3() {
  LbMotion.runLR(+500, -500);      // let Leanbot rotate right
  LbMotion.waitRotation(+1750);    // Wait for Leanbot to rotate 1750 steps (approximately 90°)
  LbMotion.runLR(-500, +500);
  LbMotion.waitRotation(-1750);
}
 
void example4() {
  LbMotion.runLR(+100, +500);      // let Leanbot turn left
  LbMotion.waitRotation(-1750);    // wait for Leanbot to turn 1750 steps (approximately 90°)
  LbMotion.runLR(-100, -500);
  LbMotion.waitRotation(+1750);
}
 
void example5() {
  LbMotion.runLR(+500, -500);      // let Leanbot rotate right
  LbMotion.waitRotationDeg(+180);  // wait for Leanbot to rotate approximately 180°
  LbMotion.runLR(-500, +500);
  LbMotion.waitRotationDeg(-180);
}
