#include "mbed.h"
#include "IridiumSBD.h"

DigitalOut myled(LED1);
Serial diagSerial(USBTX, USBRX, "diagnostic");
Serial nss(D8, D2, "isbdSerial");
IridiumSBD isbd(nss, NC);

bool ISBDCallback()
{
   myled = (millis() / 1000) % 2 == 1 ? 1 : 0;
   return true;
}

int main() {
    set_time(1256729737);
    startMillis();
    //IridiumSBD Setup
    int signalQuality = -1;

    diagSerial.baud(115200); //Serial -> mbed: Serial pc(tx, rx)
    diagSerial.printf("Hello Iridium\n");
    diagSerial.printf("hello diagnostic serial\n");
    nss.baud(19200);

    diagSerial.printf("nss baud init\n");
    
    isbd.attachConsole(diagSerial); // for diagnostic purposes
    isbd.setPowerProfile(1); // 0 for direct connect, 1 for usb connect: only changes the waiting interval in internalSendAndReceive
    diagSerial.printf("isbd setPowerProfile\n");
    isbd.begin();
    diagSerial.printf("isbd.begin()\n");
    
    // gets the signal quality by reference
    int err = isbd.getSignalQuality(signalQuality); 
    diagSerial.printf("isbd getSignalQuality\n %d", err);
    if (err != 0)
    {
      diagSerial.printf("SignalQuality failed: error ");
      diagSerial.printf("%d\n", err);
      return 1;
    }
    
    diagSerial.printf("Signal quality is ");
    diagSerial.printf("%d\n", signalQuality);
    
    err = isbd.sendSBDText("Hello, world!");
    if (err != 0)
    {
      diagSerial.printf("sendSBDText failed: error ");
      diagSerial.printf("%d\n",err);
      return 1;
    }

    diagSerial.printf("Hey, it worked!\n");
    diagSerial.printf("Messages left: ");
    diagSerial.printf("%d\n", isbd.getWaitingMessageCount());

    while(1) {
        time_t seconds = time(NULL);

        diagSerial.printf("Time as a basic string = %s", ctime(&seconds));

        myled = !myled;      
        wait(1);
    }
    return 0;
}

/*
static const int ledPin = 13;

void setup()
{
  int signalQuality = -1;

  pinMode(ledPin, OUTPUT);

  Serial.begin(115200); # Serial -> mbed: Serial pc(tx, rx)
  nss.begin(19200);

  isbd.attachConsole(Serial); # this serial port is for diagnostic purposes
  isbd.setPowerProfile(1); # 0 for direct connect, 1 for usb connect: only changes the waiting interval in internalSendAndReceive
  isbd.begin();

  int err = isbd.getSignalQuality(signalQuality); # gets the signal quality by reference
  if (err != 0)
  {
    Serial.print("SignalQuality failed: error ");
    Serial.println(err);
    return;
  }

  Serial.print("Signal quality is ");
  Serial.println(signalQuality);

  err = isbd.sendSBDText("Hello, world!");
  if (err != 0)
  {
    Serial.print("sendSBDText failed: error ");
    Serial.println(err);
    return;
  }

  Serial.println("Hey, it worked!");
  Serial.print("Messages left: ");
  Serial.println(isbd.getWaitingMessageCount());
}

void loop()
{
   digitalWrite(ledPin, HIGH);
}

bool ISBDCallback()
{
   digitalWrite(ledPin, (millis() / 1000) % 2 == 1 ? HIGH : LOW);
   return true;
}
*/
