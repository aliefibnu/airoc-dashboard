#include <Leanbot.h>
 
void setup() {
  Leanbot.begin();                // initialize Leanbot
}
 
void loop() {
  LbMission.begin(TB1A + TB1B);   // start the mission by simultaneously touching TB1A and TB1B
 
  LbMotion.runLR(1000, 1000);     // let Leanbot go straight
  LbMotion.waitDistanceMm(150);   // wait for Leanbot to reach 150mm = 15cm
  LbMotion.stopAndWait();         // stop
 
  LbMission.end();                // finish the mission
}
