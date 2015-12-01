

void logging_setup();
void logging_loop(FILE *logging_file);

void logging_setup() {
    p_sensor.MS5803Init();
    gps_ser.baud(9600);
    mkdir("/sd/test_dir", 0777); 
    FILE *fp = fopen("/sd/test_dir/test_file.txt", "a");
    if(fp == NULL) {
        error("Could not open file for write\n");
        return -1;
    } else { //Now write in one line of data into the new file! 
        update_data();
        log_data(fp);
        fclose(fp); 
    }
    mkdir("/sd/data", 0777); 
    logging_file = fopen("/sd/data/logging.txt", "a");	
}

void logging_loop(FILE *logging_file) {
    update_data();
    log_data(logging_file);        
}

void internalStateLoop();
void internalStateSetup();


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
