/*
###############################
This is the final core software program for the High Altitude Balloon Control
Project for cs241 at Stanford University.
Authors: Iskender Kushan, Shane Leonard, Bharad Raghavan, Jacky Wang
Known issues and future development:
1. Iridium module can send properly to the satellite module using both modes: 
  binary and text, but the module is currently missing the first few bytes of
  the main message received. This prevents us from doing the checksum
  verification, though also ignored in the original Arduino library.
###############################
*/

#include "mbed.h"
#include <string.h>
#include "SDFileSystem.h"
#include "TinyGPS.h"
#include "TMP102.h"
#include "MS5803.h"
#include "PID.h"
#include "Watchdog.h"
#include "ScheduleEvent.h"
#include "IridiumSBD2.h"
#include "InterruptEvent.h"

//LOGGING GLOBAL VARS
static float internal_temp = 0.0;
static float external_temp = 0.0;
static float pressure = 0.0; 
static float power = 0.0;
static float latitude = 0.0; 
static float longitude = 0.0; 
static float altitude = 0.0; 
static unsigned long precision = 0; 
static char date[32];
static unsigned long encoded_chars = 0;
static unsigned short good_sentences = 0;
static unsigned short failed_checksums = 0;
static FILE *logging_file = NULL;

//INTERNAL GLOBAL VARS
#define WATCH_DOG_RATE 22.0
#define PID_RATE 22.0
#define DESIRED_INTERNAL_TEMP 20.0 
#define CURR_TEMP 23.0
Watchdog W = Watchdog();

//IRIDIUM GLOBAL VARS 
RingBuffer<char> messageBuffer(300); //To store Iridium Messages
#define IRIDIUM_SEND_RATE 60.0 

//LOGGING PINS 
MS5803 p_sensor(D14, D15,ms5803_addrCL); 
TMP102 temperature(D14, D15, 0x90); //A0 pin is connected to ground
Serial gps_ser(D8,D2); //serial to gps, 9600 baud. D8 <-> TX , D2 <-> RX
SDFileSystem sd(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS, "sd"); // the pinout on the mbed Cool Components workshop board / mosi, miso, sclk, cs
TinyGPS gps;
AnalogIn ain(A0); //Reads the power

//INTERNAL PINS
PID controller(1.0, 0.0, 0.0, PID_RATE);
PwmOut  heater(PB_3);

//IRIDIUM PINS 
Serial diagSerial(USBTX, USBRX, "diagnostic");
Serial nss(PB_10, PB_11, "isbdSerial"); // TX, RX // PC_12, PD_2
IridiumSBD2 isbd(nss, D7); // pick a sleep pin


//GENERAL FUNCS
int complete_setup();

//LOGGING FUNCS
int logging_setup();
void logging_loop(const void *context);

//INTERNAL FUNCS
void internalStateLoop(const void *context);
void internalStateSetup();

//IRIDUM FUNCTIONS
void iridiumSetup();
void rxInterruptLoop(const void *_serial, const void *_sbd);
void iridiumLoop(const void *context);


int main() {
    //Set up a task manager that can process up to 20 tasks
    TaskManager task_manager(20);
    //Need to initialize this Interrupt before I even do setup! 
    InterruptEvent e1(&task_manager, rxInterruptLoop, &isbd, &nss);
    nss.attach(&e1, &InterruptEvent::handle);

    int setup_status = complete_setup();
    if (setup_status) {
        printf("Set up failed\n");
        return 1;
    }
    //Set up the Logging Loop
    ScheduleEvent log_event(&task_manager, logging_loop, logging_file);
    Ticker log_ticker;
    log_ticker.attach(&log_event, &ScheduleEvent::handle, 10.0);
    //Set up the Internal Loop
    ScheduleEvent internal_event(&task_manager, internalStateLoop, NULL);
    Ticker internal_ticker;
    internal_ticker.attach(&internal_event, &ScheduleEvent::handle, PID_RATE);
    //Set up Iridium SEND Loop
    ScheduleEvent iridum_send_event(&task_manager, iridiumLoop, NULL);
    Ticker iridium_send_ticker;
    iridium_send_ticker.attach(&iridum_send_event, &ScheduleEvent::handle, IRIDIUM_SEND_RATE);

    task_manager.run();

    fclose(logging_file);
    return 0;
}


int complete_setup() {

    if (W.WasResetByWatchdog()) printf("Was reset by watchdog\n");
    printf("Doing Logging Setup!\n");
    int error = logging_setup();
    if (error) return 1;
    printf("Doing Iridium Setup!\n");
    iridiumSetup();
    printf("Doing Internal Setup!\n");
    internalStateSetup();
    return 0;
}

int update_lat_long() {
    unsigned long age;
    gps.f_get_position(&latitude, &longitude, &age); //Updates longitude and latitude
    if (age == TinyGPS::GPS_INVALID_AGE) {
        latitude = -500.0; //These are sentinel values for lat long; if GPS can't track them
        longitude = -500.0; 
        return -1;
    } else {
        return 0;
    }
}

int update_datetime() {
    unsigned long age;
    int year;
    byte month, day, hour, minute, second, hundredths;
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    if (age == TinyGPS::GPS_INVALID_AGE) {
        sprintf(date,"1000 BC");
        return -1;
    } else {
        sprintf(date,"%02d/%02d/%02d %02d:%02d:%02d",month, day, year, hour, minute, second);
        return 0;
    }
}

bool gps_readable() {
    bool new_data = false;
    for (unsigned long start = time(NULL); time(NULL) - start < 0.5;) {
        while (gps_ser.readable()) {
            char c = gps_ser.getc();
            //printf("%c",c);
            if (gps.encode(c)) new_data = true;
        }
    }
    return new_data;
}


int update_data() {
    p_sensor.Barometer_MS5803(); //Gathers data from external temp/pressure sensor 
    //Updates temperatures, pressure, and power 
    pressure = p_sensor.MS5803_Pressure();
    external_temp = p_sensor.MS5803_Temperature();
    internal_temp = temperature.read();
    power = ain.read();
    //Data gathered from GPS 
    bool gps_ready = gps_readable();
    int max_gps_requests = 4;
    int gps_counter = 0;
    while (!gps_ready && gps_counter < max_gps_requests) {
        gps_ready = gps_readable();
        gps_counter++;
        //printf("Waiting!\n");
    }
    if (gps_ready) {
        update_lat_long();
        altitude = gps.f_altitude();
        precision = gps.hdop();
        gps.stats(&encoded_chars, &good_sentences, &failed_checksums);
        update_datetime();
    } else {
        //Place DUMMY GPS VALUES 
        latitude = -1000.0;
        longitude = -1000.0; 
        altitude = -1000.0; 
        precision = -1000;
        sprintf(date,"20000 BC");
        encoded_chars = -1;
        good_sentences = -1;
        failed_checksums = -1;
    }
    return 0; //Data update was a success! 
}


void log_data(FILE *fp) {
    char data_buffer[200];
    sprintf(data_buffer,"%s %.6f %.6f %.6f %lu %.6f %.6f %.6f %.6f %lu %hu %hu\n",
        date,latitude,longitude,altitude,precision,internal_temp,external_temp,pressure,power,
        encoded_chars,good_sentences,failed_checksums);
    fprintf(fp,data_buffer);
    printf(data_buffer);
}

int logging_setup() {
    p_sensor.MS5803Init();
    gps_ser.baud(9600);
    mkdir("/sd/test_dir", 0777); 
    wait(1.0);
    FILE *fp = fopen("/sd/test_dir/test_file2.txt", "a");
    wait(1.0);
    if(fp == NULL) {
        error("Could not open test file for write\n");
        return 1;
    } else { //Now write in one line of data into the new file! 
        update_data();
        log_data(fp);
        fclose(fp); 
    }
    mkdir("/sd/data", 0777); 
    wait(1.0);
    logging_file = fopen("/sd/data/logging2.txt", "a"); 
    wait(1.0);
    if(logging_file == NULL) {
        error("Could not open log file for write\n");
        return 1;
    } 
    return 0;
}

void logging_loop(const void *context) {
    FILE *log_file = (FILE *)context;
    update_data();
    log_data(log_file);        
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

void internalStateLoop(const void *context) {
    //Pet the watchdog
    W.Pet();
    controller.setProcessValue(internal_temp); //We won't actually read from the TMP 102.h, we'll use the most recent internal temp variable (global).
    // Set the new output. 
    heater = controller.compute();
    printf("What should the output be? %f\n", controller.compute());
    //printf("Was reset by watchdog? %s\n", W.WasResetByWatchdog() ? "true" : "false");
    // Now check for termination conditions
    // 1. If the GPS lat,lon exceed the permitted bounds, cut down.
    // 2. If you receive an iridum command telling you to end the flight, cut down.
    // 3. If you've not received an Iridium command in a while (5 hrs), cut down. 
}

void iridiumSetup() {
  // Initialize timer
  set_time(1256729737);
  startMillis();
  
  diagSerial.baud(115200);
  nss.baud(19200);

  isbd.attachConsole(diagSerial);
  //isbd.attachDiags(diagSerial);
  isbd.adjustATTimeout(60); // adjust to longer time out time
  isbd.attachMessageBuffer(&messageBuffer);

  isbd.begin();

  wait(5);
  isbd.setPowerProfile(1);
}

/* This function tries to send the latest logging data over Iridium */
void iridiumLoop(const void *context) {
    char data_buffer[200];
    sprintf(data_buffer,"IRIDIUM: %s %.6f %.6f %.6f %lu %.6f %.6f %.6f %.6f %lu %hu %hu",
        date,latitude,longitude,altitude,precision,internal_temp,external_temp,pressure,power,
        encoded_chars,good_sentences,failed_checksums);
    int err = isbd.sendSBDText(data_buffer);
    if (err == ISBD_BUSY) {
        diagSerial.printf("IRIDIUM IS BUSY\r\n");
        return;
    }
    if (err != 0) {
        diagSerial.printf("sendReceiveSBDText failed: error ");
        diagSerial.printf("%d\n", err);
    }

}

/* This function is invoked whenever new data is received. Such new data 
  includes both the actual message sent from as well as all control messages,
  such as "CSQ:"

  Currently, this is the function in which you may read out the actual (SBDRB) 
  message and perform appropriate tasks.
*/
void rxInterruptLoop(const void *_serial, const void *_sbd) {
  Serial *serial = (Serial *)_serial;
  IridiumSBD2 *sbd = (IridiumSBD2 *)_sbd;

  if (millis_time() - sbd->getStartTime() > 1000UL * sbd->getDuration()) {
    // Last message received timeout, and need to reset state
    // drop message
    return;
  }

  bool terminated = false; /* This is used to check if the current set of recvd
                              bytes is the end of a full control message, which
                              will trigger subsequent control actions.
                            */
  while (serial->readable()) { // serial == nss (main.cpp)
    char c = (char) serial->getc();
    sbd->console(c);
    char *prompt = sbd->getPrompt();
    char *terminator = sbd->getTerminator();

    if (prompt) {
      int matchPromptPos = sbd->getMatchPromptPos();
      switch(sbd->getPromptState()) {
        case 0: // matching prompt
          if (c == prompt[matchPromptPos]) {
            matchPromptPos++;
            sbd->setMatchPromptPos(matchPromptPos);
            if (prompt[matchPromptPos] == '\0') {
               sbd->setPromptState(1);
            }
          } else {
            matchPromptPos = c == prompt[0] ? 1 : 0; /* try to match prompt, 
                                                        if current char matches, 
                                                        then move on to next char 
                                                        to match
                                                      */
            sbd->setMatchPromptPos(matchPromptPos);
         }
         break;

        case 1: // gathering reponse from end of prompt to first
          int responseSize = sbd->getResponseSize();
          if (responseSize > 0) {
            if (c == '\r' || responseSize < 2) { 
               sbd->setPromptState(2);
            } else {
               (sbd->getRxBuffer())->insert(c); 
               // rxBuffer (only put in actual response,
               // no prompt/terminator in buffer
               responseSize--;
               sbd->setResponseSize(responseSize);
            }
          }
          break;
      }
    }
    
    if (sbd->getCompletionNum() == processSBDRBResponsNum) {
      (sbd->getRxBuffer())->insert(c);
      messageBuffer.insert(c);
    }

    // If there is no prompt, then just keep trying to match the terminator 
    // until either all terminator characters are
    // matched (return true), or no more serial to read (return false)
    int matchTerminatorPos = sbd->getMatchTerminatorPos();
    if (terminator) {
      if (c == terminator[matchTerminatorPos]) {
        matchTerminatorPos++;
        sbd->setMatchTerminatorPos(matchTerminatorPos);
        if (terminator[matchTerminatorPos] == '\0') {
          terminated = true;
        }
      } else {
        matchTerminatorPos = c == terminator[0] ? 1 : 0;
        sbd->setMatchTerminatorPos(matchTerminatorPos);
      }
    }
  } // while (serial.available())

  if (sbd->checkCompletion(terminated) && sbd->getCompletionNum() == processSBDRBResponsNum) {
    char c;
    int nBytes = sbd->getResponseSize() - 4; /* -4 because we are currently 
                                                seeing the four bytes missing
                                                on most cases, though not always
                                                consistent.
                                              */
    /* NOTE: THIS IS WHERE YOU MAY DECIDE WHAT ACTIONS/TASKS TO ENQUE FOR 
             RECEIVING DIFFERENT TYPES OF MESSAGES FROM SBDRB */
    if (nBytes > 0) {
        printf("I RECEIVED A MESSAGE :) \n");
    }
    for (int i = 0; i < nBytes; i++) {
      if (messageBuffer.pop(c)){
        diagSerial.printf("%c",c);
      }
    }
    diagSerial.printf("\r\n");
    messageBuffer.clear();
  }

  return;
}

