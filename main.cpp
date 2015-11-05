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
#include "SDFileSystem.h"
#include "TinyGPS.h"
#include "TMP102.h"
#include "MS5803.h"

float internal_temp = 0.0;
float external_temp = 0.0;
float pressure = 0.0; 
float latitude = 0.0; 
float longitude = 0.0; 
float altitude = 0.0; 
float precision = 0.0; 
char date[32];
unsigned long encoded_chars = 0;
unsigned short good_sentences = 0;
unsigned short failed_checksums = 0;


//Added from Pressure //CHANGE THE PINS FOR ONE OF THESE 
MS5803 p_sensor(PB_11, PB_10,ms5803_addrCL); 
//Added from temperature 
TMP102 temperature(PB_11, PB_10, 0x90); //A0 pin is connected to ground
//Serial gps_ser(PA_9,PA_10); //serial to gps, 9600 baud. D8 <-> TX , D2 <-> RX
Serial gps_ser(D8,D2); //serial to gps, 9600 baud. D8 <-> TX , D2 <-> RX
SDFileSystem sd(SPI_MOSI, SPI_MISO, SPI_SCK, D9, "sd"); // the pinout on the mbed Cool Components workshop board / mosi, miso, sclk, cs
///SPI_CS
TinyGPS gps;

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
    //If this works, begin the logging loop!
    mkdir("/sd/data", 0777); 
    FILE *lfp = fopen("/sd/data/logging.txt", "a");
    for (int i = 0; i < 40; i++) {
        update_data();
        log_data(lfp);
    }
    fclose(lfp);
    return 0;
}

void log_data(FILE *fp) {
    char data_buffer[200];
    sprintf(data_buffer,"%s %.6f %.6f %.6f %lu %.6f %.6f %.6f %lu %hu %hu\n",
        date,latitude,longitude,altitude,precision,internal_temp,external_temp,pressure,
        encoded_chars,good_sentences,failed_checksums);
    fprintf(fp,data_buffer);
}

int update_data() {
    p_sensor.Barometer_MS5803(); //Gathers data from external temp/pressure sensor 
    pressure = p_sensor.MS5803_Pressure();
    external_temp = p_sensor.MS5803_Temperature();
    internal_temp = temperature.read();
    unsigned long age;
    gps.f_get_position(&latitude, &longitude, &age);
    if (age == TinyGPS::GPS_INVALID_AGE) return -1;
    altitude = gps.f_altitude();
    precision = gps.hdop();
    gps.stats(&encoded_chars, &good_sentences, &failed_checksums);
    int year;
    byte month, day, hour, minute, second, hundredths;
    unsigned long age_2;
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age_2);
    if (age_2 == TinyGPS::GPS_INVALID_AGE) return -1;
    sprintf(date,"%02d/%02d/%02d %02d:%02d:%02d",month, day, year, hour, minute, second);
    return 0; //Data update was a success! 
}
