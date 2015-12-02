/*
TODO:
1. does not know if modem is alive or not, need a timer to wake up and turn 
off power

*/

#include "IridiumSBD2.h"

bool IridiumSBD2::checkCompletion(bool terminated) {
  switch (this->completionNum) {
    case powerOffNum:
      return powerOff(terminated);
    case internalAwakeNum:
      return internalAwake(terminated);
    case internalBeginATK0Num:
      return internalBeginATK0(terminated);
    case internalBeginATD0Num:
      return internalBeginATD0(terminated);
    case internalBeginATE1Num:
      return internalBeginATE1(terminated);
    case receiveSignalQualityInSendNum:
      return receiveSignalQualityInSend(terminated);
    case receiveOpenReadyNum:
      return receiveOpenReady(terminated);
    case receiveChecksumNum:
      return receiveChecksum(terminated);
    case receiveOKFromSendingSBDTextNum:
      return receiveOKFromSendingSBDText(terminated);
    case receiveSignalQualityInReceiveNum:
      return receiveSignalQualityInReceive(terminated);
    case receiveMSSTMOkToProceedNum:
      return receiveMSSTMOkToProceed(terminated);
    case receiveSBDIXResponseNum:
      return receiveSBDIXResponse(terminated);
    case receiveSBDRBResponseNum:
      return receiveSBDRBResponse(terminated);
    case processSBDRBResponsNum:
      return processSBDRBResponse(terminated);
    default:
      return false;
  }
}

// Power on the RockBLOCK or return from sleep
int IridiumSBD2::begin()
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   this->reentrant = true;
   int ret = internalBegin();
   this->reentrant = false;
   return ret;
}

// Transmit a binary message
int IridiumSBD2::sendSBDBinary(const uint8_t *txData, size_t txDataSize)
{
  if (this->reentrant) return ISBD_REENTRANT;
  
  dbg("Calling sendSBDBinary\r\n");
  dbg("initialSendOrReceive = ");
  if (this->initialSendOrReceive) {
    dbg("New send or receive\r\n");
  }else {
    dbg("Old send or receive\r\n");
  }
  dbg("delta = \r\n");
  dbg((uint16_t)((millis_time() - this->startTime)/1000));
  dbg("\r\n");
  dbg((uint16_t)(this->duration));
  if (!this->initialSendOrReceive && millis_time() - this->startTime <= 1000UL*this->duration) {
     return ISBD_BUSY;
  }
  
  this->initialSendOrReceive = false;
  this->reentrant = true;
  receiving = true;
  int ret = internalSendSBDBinary(txData, txDataSize);
  this->reentrant = false;
  return ret;
}

// Transmit a text message
int IridiumSBD2::sendSBDText(const char *txTxtMessage)
{
  if (this->reentrant) return ISBD_REENTRANT;
  
  dbg("Calling sendSBDText\r\n");
  dbg("initialSendOrReceive = ");
  if (this->initialSendOrReceive) {
    dbg("New send or receive\r\n");
  }else {
    dbg("Old send or receive\r\n");
  }
  dbg("delta = \r\n");
  dbg((uint16_t)((millis_time() - this->startTime)/1000));
  dbg("\r\n");
  dbg((uint16_t)(this->duration));
  if (!this->initialSendOrReceive && millis_time() - this->startTime <= 1000UL*this->duration) {
     return ISBD_BUSY;
  }

  this->initialSendOrReceive = false;
  this->reentrant = true;
  receiving = true;
  int ret = internalSendSBDText(txTxtMessage);
  this->reentrant = false;
  return ret;
}

// Receive a binary message
int IridiumSBD2::receiveSBDBinary()
{
  if (this->reentrant) return ISBD_REENTRANT;

  if (!this->initialSendOrReceive && millis_time() - this->startTime <= 1000UL * this->duration) {
     return ISBD_BUSY;
  }

  this->initialSendOrReceive = false;
  this->reentrant = true;
  receiving = true;
  // Sends an empty message to clear the sent message, which will trigger
  // subsequent receive message calls
  internalSendSBDText(NULL);
  this->reentrant = false;
  return ISBD_SUCCESS;
}

// High-level wrapper for AT+CSQ
int IridiumSBD2::getSignalQuality()
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   dbg(F("Calling getSignalQuality\r\n"));
   this->reentrant = true;
   int ret = internalGetSignalQuality(noneNum);
   this->reentrant = false;
   return ret;
}

// Gracefully put device to lower power mode (if sleep pin provided)
int IridiumSBD2::sleep()
{
  if (this->reentrant) {
    return ISBD_REENTRANT;
  } 

  if (this->sleepPin == NC) {
    return ISBD_NO_SLEEP_PIN;
  }

  this->reentrant = true;
  int ret = internalSleep();
  this->reentrant = false;

  return ret;
}

// Return sleep state
bool IridiumSBD2::isAsleep()
{
   return this->asleep;
}

// Return number of pending messages
int IridiumSBD2::getWaitingMessageCount()
{
   return this->remainingMessages;
}

// Define capacitor recharge times
void IridiumSBD2::setPowerProfile(int profile)
{
   switch(profile)
   {
   case 0:
      this->csqInterval = ISBD_DEFAULT_CSQ_INTERVAL;
      this->sbdixInterval = ISBD_DEFAULT_SBDIX_INTERVAL;
      break;

   case 1:
      this->csqInterval = ISBD_DEFAULT_CSQ_INTERVAL_USB;
      this->sbdixInterval = ISBD_DEFAULT_SBDIX_INTERVAL_USB;
      break;
   }
}

// Tweak AT timeout 
void IridiumSBD2::adjustATTimeout(int seconds)
{
   this->atTimeout = seconds;
}

// Tweak Send/Receive SBDIX process timeout
void IridiumSBD2::adjustSendReceiveTimeout(int seconds)
{
   this->sendReceiveTimeout = seconds;
}

// Diagnostics: attach a serial serial console
void IridiumSBD2::attachConsole(Serial &serial)
{
   this->pConsoleSerial = &serial;
}

#if ISBD_DIAGS
// Diagnostics: attach a debug console
void IridiumSBD2::attachDiags(Serial &serial)
{
   this->pDiagsSerial = &serial;
}
#endif

void IridiumSBD2::setMinimumSignalQuality(int quality)  // a number between 1 and 5, default ISBD_DEFAULT_CSQ_MINIMUM
{
  if (quality >= 1 && quality <= 5)
    this->minimumCSQ = quality;
}

void IridiumSBD2::useMSSTMWorkaround(bool useWorkAround) // true to use workaround from Iridium Alert 5/7 
{
   this->useWorkaround = useWorkAround;
}

CompletionEnum IridiumSBD2::getCompletionNum() {
  return this->completionNum;
}

// for debugging
void IridiumSBD2::setCompletionNum(CompletionEnum num) {
  this->completionNum = num;
}

int IridiumSBD2::getAtTimeout() {
  return this->atTimeout;
}

int IridiumSBD2::getMinimumCSQ() {
  return this->minimumCSQ;
}

int IridiumSBD2::getResponseSize() {
  return this->responseSize;
}

int IridiumSBD2::getPromptState() {
  return this->promptState;
}

int IridiumSBD2::getMatchPromptPos() {
  return this->matchPromptPos;
}

int IridiumSBD2::getMatchTerminatorPos() {
  return this->matchTerminatorPos;
}

char *IridiumSBD2::getPrompt() {
  return this->prompt;
}

char *IridiumSBD2::getTerminator() {
  return this->terminator;
}

unsigned long IridiumSBD2::getStartTime() {
  return this->startTime;
}

unsigned long IridiumSBD2::getDuration() {
  return this->duration;
}

uint16_t IridiumSBD2::getMtLen() {
  return this->mtLen;
}

RingBuffer<char> *IridiumSBD2::getRxBuffer() {
  return this->rxBuffer;
}

// Setter methods
void IridiumSBD2::setPromptState(int state) {
  this->promptState = state;
}

void IridiumSBD2::setMatchPromptPos(int pos) {
  this->matchPromptPos = pos;
}

void IridiumSBD2::setResponseSize(int size) {
  this->responseSize = size;
}

void IridiumSBD2::setMatchTerminatorPos(int pos) {
  this->matchTerminatorPos = pos;
}

/* 
Private interface
*/
bool IridiumSBD2::internalAwake(bool terminated) {
  if (!terminated) {
    return false;
  }

  dbg(F("Calling internalAwake\r\n"));
  this->asleep = false;
  return true;
}

bool IridiumSBD2::internalBeginATK0(bool terminated) {
  if (!terminated) {
    return false; // keep waiting for more data
  }

  dbg(F("Calling internalBeginATK0\r\n"));
  send(F("AT&K0\r"));
  startTime = millis_time();
  duration = atTimeout;
  waitForATResponse(internalAwakeNum);

  return true;
}

bool IridiumSBD2::internalBeginATD0(bool terminated) {
  if (!terminated) {
    return false; // keep waiting for more data
  }

  dbg(F("Calling internalBeginATD0\r\n"));
  send(F("AT&D0\r"));
  startTime = millis_time();
  duration = atTimeout;
  waitForATResponse(internalBeginATK0Num);

  return true;
}

bool IridiumSBD2::internalBeginATE1(bool terminated) {
  if (!terminated) {
    return false; // keep waiting for more data
  }
  
  dbg(F("Calling internalBeginATE1\r\n"));
  send(F("ATE1\r"));
  startTime = millis_time();
  duration = atTimeout;
  waitForATResponse(internalBeginATD0Num);

  return true;
}

int IridiumSBD2::internalBegin()
{
  dbg(F("Calling internalBegin\r\n"));

  if (!this->asleep) {
    return ISBD_ALREADY_AWAKE;
  }
  
  power(true); // power on
   
  uint32_t startupTime = 500; //ms
  for (uint32_t start = millis_time(); millis_time() - start < startupTime;) {
    /* Future: make this a non-blocking wait (low priority)*/
  }
  
  // Turn on modem and wait for a response from "AT" command to begin
  this->asleep = true; // Jacky's trick to make all other internal functions inactive
                       // except for begin() and waitForATResponse to accept data
  startTime = millis_time();
  duration = ISBD_STARTUP_MAX_TIME;
  send(F("AT\r"));
  waitForATResponse(internalBeginATE1Num);
  return ISBD_SUCCESS;
}

int IridiumSBD2::internalSendSBDText(const char *txTxtMessage) {
  if (this->asleep) {
    return ISBD_IS_ASLEEP;
  }
 
  dbg(F("internalSendText\r\n"));

  send(F("AT+SBDWT="), true, false);
  if (txTxtMessage) {
    // It's ok to have a NULL txtTxtMessage if the transaction is RX only
    send(txTxtMessage);
  }
  send(F("\r"), false);
 
  this->startTime = millis_time();
  this->duration = atTimeout;
  waitForATResponse(receiveOKFromSendingSBDTextNum);

  return ISBD_SUCCESS;
}

int IridiumSBD2::internalSendSBDBinary(const uint8_t *txData, size_t txDataSize)
{
  if (this->asleep) {
    return ISBD_IS_ASLEEP;
  }
 
  dbg(F("internalSendBinary\r\n")); // F(s) does casting to __FlashStringHelper*
  
  // Copy txData to software buffer
  this->txBuffer->clear();
  for (size_t i = 0; i < txDataSize; i++) {
    char c = (char)txData[i];
    if (!this->txBuffer->insert(c)) {
      dbg("TX Buffer full!\r\n");
    }
  }
  this->txDataSize = txDataSize;

  // Get Signal Quality
  this->startTime = millis_time();
  this->duration = atTimeout;
  internalGetSignalQuality(receiveSignalQualityInSendNum);

  // after get signal quality -> sendOpenconnectionRequest -> sendData
  return ISBD_SUCCESS;
}

int IridiumSBD2::internalReceiveSBD()
{
   dbg(F("internalReceive\r\n")); // F(s) does casting to __FlashStringHelper*

   if (this->asleep) {
      return ISBD_IS_ASLEEP;
   }

   this->startTime = millis_time();
   this->duration = ISBD_DEFAULT_SENDRECEIVE_TIME;
   this->quality = 0;

   // Get Signal Quality
   internalGetSignalQuality(receiveSignalQualityInReceiveNum);

   // after signal quality -> MSSTM -> doSBDIX -> doSBDRB
   return ISBD_SUCCESS;
}

/* function called by the task function to receive data */
int IridiumSBD2::internalGetSignalQuality(CompletionEnum completionNum)
{
   if (this->asleep)
      return ISBD_IS_ASLEEP;

   send(F("AT+CSQ\r"));
   int csqResponseSz = 2;
   waitForATResponse(completionNum, csqResponseSz, "+CSQ:");
   return ISBD_SUCCESS;
}

/* function called in UART interrupt hanlder */
void IridiumSBD2::internalMSSTMWorkaround() {
   int msstmResponseSz = 24;
   send(F("AT-MSSTM\r"));
   waitForATResponse(receiveMSSTMOkToProceedNum, msstmResponseSz, "-MSSTM: ");
}

bool IridiumSBD2::powerOff(bool terminated) {
  if (!terminated) {
    return false;
  }

  power(false);
  return true;
}

int IridiumSBD2::internalSleep()
{
   if (this->asleep)
      return ISBD_IS_ASLEEP;

   // Best Practices Guide suggests this before shutdown
   send(F("AT*F\r"));

   this->startTime = millis_time();
   this->duration = atTimeout;
   waitForATResponse(powerOffNum);

   return ISBD_SUCCESS;
}

// Wait for response from previous AT command.
// This function sets the completion function that:
//  returns false if the received bytes in rxBuffer is not complete
//  returns true if the received bytes in rxBuffer is complete after executing subsequent commands
void IridiumSBD2::waitForATResponse(CompletionEnum completionNum,
                                   int responseSize,
                                   char *prompt, 
                                   char *terminator) {
  this->rxBuffer->clear(); // clear entire rx buffer
  this->prompt = prompt;
  this->terminator = terminator;
  this->promptState = prompt ? 0 : 2;
  this->matchPromptPos = 0; // Matches chars in prompt
  this->matchTerminatorPos = 0; // Matches chars in terminator
  this->responseSize = responseSize;
  this->completionNum = completionNum;
  console(F("<< "));
}

int IridiumSBD2::doSBDIX() {
   // xx, xxxxx, xx, xxxxx, xx, xxx
   int sbdixResponseBufSz = 32;
   send(F("AT+SBDIX\r"));
   waitForATResponse(receiveSBDIXResponseNum, sbdixResponseBufSz, "+SBDIX: ");
   return ISBD_SUCCESS;
}

int IridiumSBD2::doSBDRB() {
   send(F("AT+SBDRB\r"));
   waitForATResponse(receiveSBDRBResponseNum, 0, NULL, "AT+SBDRB\r\n"); // waits for its own echo
   return ISBD_SUCCESS;
}

void IridiumSBD2::send(const char *str, bool beginLine, bool endLine)
{
   if (beginLine)
      console(F(">> "));
   console(str);
   if (endLine)
      console(F("\r\n"));
   serial.printf("%s\n",str);
}

void IridiumSBD2::send(uint16_t n)
{
   console(n);
   serial.printf("%u",n);
}

bool IridiumSBD2::receiveSignalQualityInSend(bool terminated) {
  if (!terminated) {
    return false;
  }
  
  dbg(F("Calling receiveSignalQualityInSend\r\n"));
  char quality_char;
  rxBuffer->pop(quality_char);
  if (isdigit(quality_char))
  {
    quality = quality_char - '0';
    dbg(F("Signal Quality is "));
    dbg((uint16_t)quality);
    dbg("\r\n");
    if (quality >= minimumCSQ) {
      /* Send Open Connection Request*/
      internalSendOpenConnectionRequest();
      return true;
    }
  }
  
  /* Retry on getting CSQ */
  internalGetSignalQuality(receiveSignalQualityInSendNum);
  return true;
}

void IridiumSBD2::internalSendOpenConnectionRequest() {
  send("AT+SBDWB=", true, false);
  send(txDataSize);
  send("\r", false);

  /* wait for ready signal */
  this->startTime = millis_time();
  this->duration = atTimeout;
  waitForATResponse(receiveOpenReadyNum, 0, NULL, "READY\r\n");
}

bool IridiumSBD2::receiveOpenReady(bool terminated) {
  if (!terminated) {
    return false;
  }
  
  dbg(F("Calling receiveOpenReady\r\n"));
  uint16_t checksum = 0;
  for (unsigned int i=0; i < txDataSize; ++i)
  { 
    char c;
    if (!txBuffer->pop(c)) {
      dbg("receiveOpenRead: txBuffer has less data than txDataSize\r\n");
      break;
    }
    dbg((uint16_t)c);
    serial.putc(c);
    checksum += (uint16_t)c;
  }

  console(F("["));
  console((uint16_t)txDataSize); 
  console(F(" bytes]"));
  dbg(F("Checksum:"));
  dbg(checksum);
  dbg(F("\r\n"));

  serial.putc(checksum >> 8);
  serial.putc(checksum & 0xFF);

  /* wait for checksum ok confirmation */
  startTime = millis_time();
  duration = atTimeout; 
  waitForATResponse(receiveChecksumNum, 0, NULL, "0\r\n\r\nOK\r\n");
  return true;
}

bool IridiumSBD2::receiveChecksum(bool terminated) {
  if (!terminated) {
    return false;
  }

  dbg(F("Calling receiveChecksum\r\n"));

  // Need to perform receive operations to fully send the text
  internalReceiveSBD();

  return true;
}

bool IridiumSBD2::receiveOKFromSendingSBDText(bool terminated) {
  if (!terminated) {
    return false;
  }

  dbg(F("Calling receiveOKFromSendingSBDText\r\n"));

  internalReceiveSBD();
  return true;
}

bool IridiumSBD2::receiveSignalQualityInReceive(bool terminated) {
  if (!terminated) {
    return false;
  }
  
  dbg(F("Calling receiveSignalQualityInReceive\r\n"));
  char quality_char;
  rxBuffer->pop(quality_char);
  if (isdigit(quality_char))
  {
    quality = quality_char - '0';
    dbg(F("Signal Quality is "));
    dbg((uint16_t)quality);
    dbg("\r\n");

    if (quality >= minimumCSQ) {

      /* Always Send MSSTM */
      internalMSSTMWorkaround();
      return true;
    }
  } 
  
  /* Retry on getting CSQ */
  internalGetSignalQuality(receiveSignalQualityInReceiveNum);
  return true;
}

bool IridiumSBD2::receiveMSSTMOkToProceed(bool terminated) {
  if (!terminated) {
    return false;
  }

  char msstm_response_char;
  rxBuffer->pop(msstm_response_char);
  // Response buf now contains either an 8-digit number or the string "no network service"
  bool okToProceed = isdigit(msstm_response_char);
  if (okToProceed && quality >= minimumCSQ) {
    doSBDIX();
    return true;
  }

  /* Retry on getting CSQ */
  internalGetSignalQuality(receiveSignalQualityInReceiveNum);
  return true;
}

bool IridiumSBD2::receiveSBDIXResponse(bool terminated) {
  if (!terminated) {
    return false;
  }
  
  dbg(F("Calling receiveSBDIXResponse\r\n"));

  char sbdixResponseBuf[32];
  for (int i = 0; i < 32; i++) {
    if (!rxBuffer->pop(sbdixResponseBuf[i])) {
      /* No more data in buffer */
      break;
    }
  }

  uint16_t *values[6] = { &(moCode), 
                          &(moMSN), 
                          &(mtCode), 
                          &(mtMSN), 
                          &(mtLen), 
                          &(mtRemaining) };
  for (int i = 0; i < 6; i++)
  {
    char *p = strtok(i == 0 ? sbdixResponseBuf : NULL, ", ");
    if (p == NULL) {
      return true;
    }
    *values[i] = atol(p);
  }

  /* Successfully gotten SBDIX meta data for receiving message */

  dbg(F("SBDIX MO code: "));
  dbg(moCode);
  dbg(F("\r\n"));

  if (moCode <= 4) // successful return!
  {
    dbg(F("SBDIX success!\r\n"));

    remainingMessages = mtRemaining;
    if (mtCode == 1 && receiving) // retrieved 1 message
    {
      dbg(F("Incoming message\r\n"));
      doSBDRB();
    }  
  } else if (moCode == 12 || moCode == 14 || moCode == 16) { // fatal failure: no retry
    dbg(F("SBDIX fatal!\r\n"));
  } else { // retry
    dbg(F("Waiting for SBDIX retry...\r\n"));
    // TODO: wait for sbdixInterval seconds before doSBDIX again
    doSBDIX();
  }

  return true;
}

bool IridiumSBD2::receiveSBDRBResponse(bool terminated) {
  if (!terminated) {
    return false;
  }
  
  dbg(F("Calling receiveSBDRBResponse\r\n"));
  
  /* Reinitialize timing for actual data receiving */
  startTime = millis_time();
  duration = atTimeout;
  waitForATResponse(processSBDRBResponsNum, mtLen + 5, NULL, NULL); // No terminator as well!
  return true;
}

bool IridiumSBD2::processSBDRBResponse(bool terminated) {
  if (responseSize >= 2) {
    responseSize--;
    //dbg("processSBDRB: responseSize = ");
    //dbg((uint16_t)this->responseSize);
    //dbg("\r\n");
    return false;
  }

  dbg(F("Calling processSBDRBResponse\r\n"));
  dbg((uint16_t)rxBuffer->getNumElem());
  dbg("\r\n");
  /*
  char szByte1 = 0;
  char szByte2 = 0;
  rxBuffer->pop(szByte1);
  rxBuffer->pop(szByte2);
  uint16_t size = 256 * (uint8_t)szByte1 + (uint8_t)szByte2;

  console(F("[Binary size:"));
  console(size);
  console(F("]"));
  
  for (int i = 0; i < this->mtLen; i++) {
    char c;
    if (rxBuffer->pop(c)) {
      dbg(c);
      messageBuffer->insert(c);
    } else {
      dbg("processSBDRBResponse Not enough data received\n");
    }
  }
*/
  /*char cksumByte1 = 0;
  char cksumByte2 = 0;
  rxBuffer->pop(cksumByte1);
  rxBuffer->pop(cksumByte2);

  uint16_t checksum = 256 * (uint8_t)cksumByte1 + (uint8_t)cksumByte2;
  console(F("[csum:"));
  console(checksum);
  console(F("]"));
  
  this->completionNum = noneNum;
  this->duration = 0; // allow any requests for send/receive since entire message received
  */
  /* TODO: schedule task to process data */

  return true;
}

/************** NON-Time Sensitive Code ***************/
void IridiumSBD2::power(bool on)
{
   static unsigned long lastPowerOnTime = 0UL;

   if (this->sleepPin == NC)
      return;

   if (on)
   {
      dbg(F("Powering on RockBLOCK...!\r\n"));
      this->asleep = false;
      //pinMode(this->sleepPin, OUTPUT);
      DigitalOut sleep_pin(this->sleepPin);
      //digitalWrite(this->sleepPin, HIGH); // HIGH = awake
      sleep_pin = 1;
      lastPowerOnTime = millis_time();
   }

   else
   {
      // Best Practices Guide suggests waiting at least 2 seconds
      // before powering off again
      unsigned long elapsed = millis_time() - lastPowerOnTime;
      if (elapsed < 2000UL)
         wait(elapsed/1000);

      dbg(F("Powering off RockBLOCK...!\r\n"));
      this->asleep = true;
      //pinMode(this->sleepPin, OUTPUT);
      DigitalOut sleep_pin(this->sleepPin);
      //digitalWrite(this->sleepPin, LOW); // LOW = asleep
      sleep_pin = 0;
   }
}

void IridiumSBD2::attachMessageBuffer(RingBuffer<char> *messageBuffer) {
  this->messageBuffer = messageBuffer; 
}

void IridiumSBD2::dbg(const char *msg) {
  #if ISBD_DIAGS
  if (this->pDiagsSerial)
    pDiagsSerial->printf("%s",msg);
  #endif
}

void IridiumSBD2::dbg(uint16_t n) {
  #if ISBD_DIAGS
  if (this->pDiagsSerial)
    pDiagsSerial->printf("%u",n);
  #endif
}

void IridiumSBD2::dbg(char c) {
  #if ISBD_DIAGS
  if (this->pDiagsSerial)
    pDiagsSerial->printf("%c",c);
  #endif
}

void IridiumSBD2::console(const char *msg) {
  if (this->pConsoleSerial)
    pConsoleSerial->printf("%s",msg);
}

void IridiumSBD2::console(char c) {
  if (this->pConsoleSerial)
    pConsoleSerial->printf("%c",c);
}

void IridiumSBD2::console(uint16_t n) {
  if (this->pConsoleSerial)
    pConsoleSerial->printf("%u",n);
}

