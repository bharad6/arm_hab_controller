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

//Serial gps_ser(PA_9,PA_10); //serial to gps, 9600 baud. D8 <-> TX , D2 <-> RX
Serial gps_ser(D8,D2); //serial to gps, 9600 baud. D8 <-> TX , D2 <-> RX
SDFileSystem sd(SPI_MOSI, SPI_MISO, SPI_SCK, D9, "sd"); // the pinout on the mbed Cool Components workshop board / mosi, miso, sclk, cs
///SPI_CS
TinyGPS gps;

//function definitions
static void print_date(FILE *fp, TinyGPS &gps);

int main() {
    printf("Hello World!\n");   
 
    mkdir("/sd/mydir2", 0777);
    
    FILE *fp = fopen("/sd/filetest.txt", "a");
    if(fp == NULL) {
        error("Could not open file for write\n");
    }
    fprintf(fp, "Hello fun SD Card World!"); 
    printf("Goodbye World!\n");
    fclose(fp); 
    
    gps_ser.baud(9600);
    printf("Simple TinyGPS library v. %i\n",TinyGPS::library_version());
    while(1)
    {
        bool newData = false;
        unsigned long chars;
        unsigned short sentences, failed;

        // For one second we parse GPS data and report some key values
        for (unsigned long start = time(NULL); time(NULL) - start < 1;)
        {
            //int i = 0;
            while (gps_ser.readable())
            {
                char c = gps_ser.getc();
                //printf("%c",c); // uncomment this line if you want to see the GPS data flowing
                if (gps.encode(c)) {
                    newData = true;// Did a new valid sentence come in?   
                    //printf("The Data is TRUE\n");
                }
                //i+=1;
            }
            //printf("Num of chars: %d\n",i);
        }
        FILE *fp = fopen("/sd/filetest.txt", "a");
        if (newData)
        {
            float flat, flon;
            unsigned long age;
            gps.f_get_position(&flat, &flon, &age);
            print_date(fp, gps);
            printf("LAT=%.6f LON=%.6f ALT=%.6f PREC= %i\n",flat,flon,gps.f_altitude(),gps.hdop());
            //printf("LAT=%.6f");
            //printf("%.6f",flat);
            fprintf(fp,"LAT= %.6f",flat);
            //printf(" LON=");
            //printf("%.6f",flon);
            fprintf(fp," LON= %.6f",flon);
            //printf(" ALT=");
            //printf("%.6f",gps.f_altitude());
            fprintf(fp," ALT= %.6f",gps.f_altitude());
            //printf(" PREC= %i", gps.hdop());
            fprintf(fp," PREC= %i", gps.hdop());
            //printf("\n");
        }
  
        gps.stats(&chars, &sentences, &failed);
        printf("CHARS=%u SENTENCES=%u CSUM_ERR=%u\n",chars,sentences,failed);
        //printf(" CHARS=");
        //printf("%u",chars);
        fprintf(fp," CHARS = %u",chars);
        
        //printf(" SENTENCES=");
        //printf("%u",sentences);
        fprintf(fp," SENTENCES = %u",sentences);
        
        //printf(" CSUM ERR=");
        //printf("%u\n",failed);
        fprintf(fp," CSUM ERR = %u\n",failed);
        fclose(fp);
        if (chars == 0) printf("** No characters received from GPS: check wiring **\n");
    }   
    
}

static void print_date(FILE *fp, TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE){
    printf("********** ******** ");
    fprintf(fp,"********** ******** ");
  }
  else
  {
    char sz[32];
    sprintf(sz, "DATE&TIME = %02d/%02d/%02d %02d:%02d:%02d\n",
        month, day, year, hour, minute, second);
    printf(sz);
    fprintf(fp,sz);
  }
}