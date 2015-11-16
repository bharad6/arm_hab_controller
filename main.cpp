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


MS5803 p_sensor(D14, D15,ms5803_addrCL); 
TMP102 temperature(D14, D15, 0x90); //A0 pin is connected to ground
Serial gps_ser(D8,D2); //serial to gps, 9600 baud. D8 <-> TX , D2 <-> RX
SDFileSystem sd(SPI_MOSI, SPI_MISO, SPI_SCK, D9, "sd"); // the pinout on the mbed Cool Components workshop board / mosi, miso, sclk, cs
///SPI_CS
TinyGPS gps;
AnalogIn ain(A0);

int update_data();
void log_data(FILE *fp);


int main() {
    //Initialize relevant sensors 
    p_sensor.MS5803Init();
    gps_ser.baud(9600);
    //printf("Simple TinyGPS library v. %i\n",TinyGPS::library_version());
    //Test out SD Card library's functionality 
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
    printf("Now beginning logging loop!\n");
    //If this works, begin the logging loop!
    mkdir("/sd/data", 0777); 
    FILE *lfp = fopen("/sd/data/logging.txt", "a");
    while (true) {
        update_data();
        log_data(lfp);        
    }
    fclose(lfp);
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
        printf("Waiting!\n");
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
