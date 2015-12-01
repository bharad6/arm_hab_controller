/*
###############################
mbed nucleo l152re sd and gps test
using tinygps v13 and sdfilesystem libraries.
last change: 10/12/2015
status:
    - gps working
    - sd logging functional

###############################
*/

#include "mbed.h"
#include <string>
#include "SDFileSystem.h"
#include "TinyGPS.h"
#include "TMP102.h"
#include "MS5803.h"
#include "PID.h"
#include "Watchdog.h"
#include "ScheduleEvent.h"


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
#define WATCH_DOG_RATE 500.0
#define PID_RATE 120.0
#define DESIRED_INTERNAL_TEMP 20.0 
#define CURR_TEMP 23.0
Watchdog W = Watchdog();

//LOGGING PINS 
MS5803 p_sensor(D14, D15,ms5803_addrCL); 
TMP102 temperature(D14, D15, 0x90); //A0 pin is connected to ground
Serial gps_ser(D8,D2); //serial to gps, 9600 baud. D8 <-> TX , D2 <-> RX
SDFileSystem sd(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS, "sd"); // the pinout on the mbed Cool Components workshop board / mosi, miso, sclk, cs
TinyGPS gps;
AnalogIn ain(A0); //Reads the power

//INTERNAL PINS
PID controller(1.0, 0.0, 0.0, PID_RATE);
PwmOut  heater(PB_10);

//LOGGING FUNCS
int logging_setup();
void logging_loop(FILE *logging_file);
int update_data();
void log_data(FILE *fp);

//INTERNAL FUNCS
void internalStateLoop();
void internalStateSetup();


int main() {
    printf("Doing Logging Setup!\n");
    int error = logging_setup();
    if (error) return -1;
    printf("Now beginning the logging loop!\n");
    while (true) {
        logging_loop(logging_file); 
    }
    fclose(logging_file);
    return 0;
}

void log_data(FILE *fp) {
    char data_buffer[200];
    sprintf(data_buffer,"%s %.6f %.6f %.6f %lu %.6f %.6f %.6f %.6f %lu %hu %hu\n",
        date,latitude,longitude,altitude,precision,internal_temp,external_temp,pressure,power,
        encoded_chars,good_sentences,failed_checksums);
    fprintf(fp,data_buffer);
    printf(data_buffer);
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

void place_dummy_gps_values() {
    latitude = -1000.0;
    longitude = -1000.0; 
    altitude = -1000.0; 
    precision = -1000;
    sprintf(date,"20000 BC");
    encoded_chars = -1;
    good_sentences = -1;
    failed_checksums = -1;
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
        place_dummy_gps_values();
    }
    return 0; //Data update was a success! 
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


void logging_loop(FILE *log_file) {
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
