/*
IridiumSBD - An Arduino library for Iridium SBD ("Short Burst Data") Communications
Suggested and generously supported by Rock Seven Location Technology
(http://rock7mobile.com), makers of the brilliant RockBLOCK satellite modem.
Copyright (C) 2013 Mikal Hart
All rights reserved.

The latest version of this library is available at http://arduiniana.org.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "mbed.h"
#include "millis.h"
#include "ctype.h"
#include "RingBuffer.h"

typedef bool (*Completion)(const bool);

typedef enum {
  noneNum,
  powerOffNum,
  internalAwakeNum,
  internalBeginATK0Num,
  internalBeginATD0Num,
  internalBeginATE1Num,
  receiveSignalQualityInSendNum,
  receiveOpenReadyNum,
  receiveChecksumNum,
  receiveOKFromSendingSBDTextNum,
  receiveSignalQualityInReceiveNum,
  receiveMSSTMOkToProceedNum,
  receiveSBDIXResponseNum,
  receiveSBDRBResponseNum,
  processSBDRBResponsNum,
} CompletionEnum;


// To replace the Wstring library, which casts the string
#define F(str) (str)

#define ISBD_LIBRARY_REVISION           1

#define ISBD_DIAGS                      1

#define ISBD_DEFAULT_AT_TIMEOUT         30
#define ISBD_DEFAULT_CSQ_INTERVAL       10
#define ISBD_DEFAULT_CSQ_INTERVAL_USB   20
#define ISBD_DEFAULT_SBDIX_INTERVAL     30
#define ISBD_DEFAULT_SBDIX_INTERVAL_USB 30
#define ISBD_DEFAULT_SENDRECEIVE_TIME   300
#define ISBD_STARTUP_MAX_TIME           240
#define ISBD_DEFAULT_CSQ_MINIMUM        2

#define ISBD_SUCCESS             0
#define ISBD_ALREADY_AWAKE       1
#define ISBD_SERIAL_FAILURE      2
#define ISBD_PROTOCOL_ERROR      3
#define ISBD_CANCELLED           4
#define ISBD_NO_MODEM_DETECTED   5
#define ISBD_SBDIX_FATAL_ERROR   6
#define ISBD_SENDRECEIVE_TIMEOUT 7
#define ISBD_RX_OVERFLOW         8
#define ISBD_REENTRANT           9
#define ISBD_IS_ASLEEP           10
#define ISBD_NO_SLEEP_PIN        11
#define ISBD_BUSY                12

#define ISBD_MAX_SOFTWARE_BUFFER_SZ 500 // maximum number of bytes buffered from hardware to software

extern bool ISBDCallback() __attribute__ ((weak)); /*  ISBDCallback is to a callback function that allows the 
program to keep running some code while isbd tries to send message, which can take few minutes */

class IridiumSBD2 {
public:
  // Communication methods
  int begin();
  int sendSBDBinary(const uint8_t *txData, size_t txDataSize);
  int sendSBDText(const char *txTxtMessage);
  int receiveSBDBinary();
  int getSignalQuality(); // forces iridium to acquire signal quality with satellite

  // State checking functions
  int getWaitingMessageCount();
  int sleep();
  bool isAsleep();

  // Setup functions
  void adjustATTimeout(int seconds);          // default value = 20 seconds
  void adjustSendReceiveTimeout(int seconds); // default value = 300 seconds
  void attachMessageBuffer(RingBuffer<char> *messageBuffer); // attach received message buffer in main controller
  void setMinimumSignalQuality(int quality);  // a number between 1 and 5, default ISBD_DEFAULT_CSQ_MINIMUM
  void setPowerProfile(int profile);          // 0 = direct connect (default), 1 = USB
  void useMSSTMWorkaround(bool useWorkAround); // true to use workaround from Iridium Alert 5/7 

  void attachConsole(Serial &serial);
  #if ISBD_DIAGS
  void attachDiags(Serial &serial);
  #endif

  // Getter methods
  int getAtTimeout();
  int getMinimumCSQ();
  int getResponseSize();
  int getPromptState();
  int getMatchPromptPos();
  int getMatchTerminatorPos();
  CompletionEnum getCompletionNum();
  void setCompletionNum(CompletionEnum num); //for debugging
  bool processSBDRBResponse(bool terminated); //for debugging making this public

  char *getPrompt();
  char *getTerminator();

  unsigned long getStartTime();
  unsigned long getDuration();

  uint16_t getMtLen();
  
  RingBuffer<char> *getRxBuffer();

  // Setter methods
  void setPromptState(int state);
  void setMatchPromptPos(int pos);
  void setResponseSize(int size);
  void setMatchTerminatorPos(int pos);

  /* Constructor */
  IridiumSBD2(Serial &str, PinName sleepPinName = NC) : 
    serial(str),
    pConsoleSerial(NULL),
    #if ISBD_DIAGS
    pDiagsSerial(NULL),
    #endif
    csqInterval(ISBD_DEFAULT_CSQ_INTERVAL),
    sbdixInterval(ISBD_DEFAULT_SBDIX_INTERVAL),
    atTimeout(ISBD_DEFAULT_AT_TIMEOUT),
    sendReceiveTimeout(ISBD_DEFAULT_SENDRECEIVE_TIME),
    remainingMessages(-1),
    asleep(true),
    reentrant(false),
    sleepPin(sleepPinName),
    minimumCSQ(ISBD_DEFAULT_CSQ_MINIMUM),
    useWorkaround(true) {
      DigitalOut sleep_pin(sleepPin);

      /* Transmitting & Receiving State Initializer */
      rxBuffer = new RingBuffer<char>(ISBD_MAX_SOFTWARE_BUFFER_SZ);
      txDataSize = 0;
      txBuffer = new RingBuffer<char>(ISBD_MAX_SOFTWARE_BUFFER_SZ);
      startTime = 0;
      duration = 0;
      initialSendOrReceive = true;
      quality = -1;
      prompt = NULL;
      terminator = NULL;
      promptState = 0;
      matchPromptPos = 0;
      matchTerminatorPos = 0;
      responseSize = 0;
      completionNum = noneNum;
      moCode = 0;
      moMSN = 0;
      mtCode = 0;
      mtMSN = 0;
      mtLen = 0; 
      mtRemaining = 0;
      receiving = false;
    }
  
  void dbg(const char *str);
  void dbg(uint16_t n);
  void dbg(char c);

  void console(const char* str); // prints to the stream that is attached
  void console(uint16_t n);
  void console(char c);

  bool checkCompletion(bool terminated);

private:
  Serial &serial; // serial is set up at construction time of the iridiumSBD object
  Serial *pConsoleSerial; // serial attached after via attachConsole -> this is for diagnostic
  #if ISBD_DIAGS
  Serial *pDiagsSerial;
  #endif

  // Timings
  int csqInterval;
  int sbdixInterval;
  int atTimeout;
  int sendReceiveTimeout;

  // State variables  
  int remainingMessages;
  bool asleep;
  bool reentrant;
  PinName sleepPin;
  int minimumCSQ;
  bool useWorkaround;
  
  // Communication states
  RingBuffer<char> *messageBuffer = NULL; // to be attached by the main client

  // States for receiving messages 
  RingBuffer<char> *txBuffer; // temporary software buffer for tx
  unsigned int txDataSize; // number of bytes to send

  RingBuffer<char> *rxBuffer; // temporary software buffer for rx
  int quality; // signal quality
  unsigned long startTime;
  unsigned long duration; // in seconds
  bool initialSendOrReceive; // initial sending or receiving that does not depend on startTime+duration to see if iridium is busy with other process
  char *prompt; // default
  char *terminator; // default
  bool receiving; // current message transmission is for receiving
  int promptState; // = prompt ? 0 : 2;
  int matchPromptPos;
  int matchTerminatorPos;
  int responseSize;
  CompletionEnum completionNum;
  uint16_t moCode, moMSN, mtCode, mtMSN, mtLen, mtRemaining;

  // Internal utilities
  void waitForATResponse(CompletionEnum completionNum, 
                         int responseSize=0,
                         char *prompt=NULL,
                         char *terminator="OK\r\n");

  int internalBegin();
  int internalSendSBDBinary(const uint8_t *txData, size_t txDataSize); 
  int internalSendSBDText(const char *txTxtMessage);
  void internalSendOpenConnectionRequest();
  int internalReceiveSBD();
  int internalGetSignalQuality(CompletionEnum completionNum);
  void internalMSSTMWorkaround();
  int internalSleep();


  /* internalBegin sequence of callbacks */
  bool internalBeginATE1(bool terminated);
  bool internalBeginATD0(bool terminated);
  bool internalBeginATK0(bool terminated);
  bool internalAwake(bool terminated);
  bool receiveOKFromSendingSBDText(bool terminated);
  
  /* internalSend sequence of callbacks */
  bool receiveSignalQualityInSend(bool terminated);
  bool receiveOpenReady(bool terminated);
  bool receiveChecksum(bool terminated);

  /* internalReceive sequence of callbacks */
  bool receiveMSSTMOkToProceed(bool terminated);
  bool receiveSignalQualityInReceive(bool terminated);
  bool receiveSBDIXResponse(bool terminated);
  bool receiveSBDRBResponse(bool terminated);
  //bool processSBDRBResponse(bool terminated);

  /* internalSleep sequence of callbacks */
  bool powerOff(bool terminated);

  int doSBDIX();
  int doSBDRB(); 
  void power(bool on);

  void send(const char* str, bool beginLine = true, bool endLine = true);
  void send(uint16_t n);

  bool cancelled();
};
