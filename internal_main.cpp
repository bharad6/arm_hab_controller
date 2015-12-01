#include "PID.h"
#include "Watchdog.h"

#define WATCH_DOG_RATE 500.0
#define PID_RATE 120.0
#define DESIRED_INTERNAL_TEMP 20.0 
#define CURR_TEMP 23.0

void internalStateLoop();
void internalStateSetup();

Watchdog W = Watchdog();
PID controller(1.0, 0.0, 0.0, PID_RATE);
PwmOut  heater(PB_10);

int main(){
  internalStateSetup();
  printf("Starting the program!\n");
  for (int i = 0; i < 5; i++ ) {
    internalStateLoop();
    //wait(WATCH_DOG_RATE);
  }

}

void internalStateSetup() {
  //TMP102.h temperature ranges from -40 to 125 C
  controller.setInputLimits(-40.0, 125.0);
  controller.setOutputLimits(0.0, 1.0); 
  controller.setBias(0.3); // Try experimenting with the bias! 
  controller.setMode(AUTO_MODE);
  controller.setSetPoint(DESIRED_INTERNAL_TEMP);  
  W.Start(WATCH_DOG_RATE); 
}

void internalStateLoop() {
    //Pet the watchdog
    W.Pet();
    controller.setProcessValue(CURR_TEMP); //We won't actually read from the TMP 102.h, we'll use the most recent internal temp variable (global).
    // Set the new output. 
    heater = controller.compute();
    printf("What should the output be? %f\n", controller.compute());
    //printf("Was reset by watchdog? %s\n", W.WasResetByWatchdog() ? "true" : "false");
    // Now check for termination conditions
    // 1. If the GPS lat,lon exceed the permitted bounds, cut down.
    // 2. If you receive an iridum command telling you to end the flight, cut down.
    // 3. If you've not received an Iridium command in a while (5 hrs), cut down. 
}
