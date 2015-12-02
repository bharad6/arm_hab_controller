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

#include "IridiumSBD.h"

// Power on the RockBLOCK or return from sleep
int IridiumSBD::begin()
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   this->reentrant = true;
   int ret = internalBegin();
   this->reentrant = false;

   // Absent a successful startup, keep the device turned off
   if (ret != ISBD_SUCCESS) {
      power(false);
      console("power off\n");
   }

   return ret;
}

// Transmit a binary message
int IridiumSBD::sendSBDBinary(const uint8_t *txData, size_t txDataSize)
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   this->reentrant = true;
   int ret = internalSendReceiveSBD(NULL, txData, txDataSize, NULL, NULL);
   this->reentrant = false;
   return ret;
}

// Transmit and receive a binary message
int IridiumSBD::sendReceiveSBDBinary(const uint8_t *txData, size_t txDataSize, uint8_t *rxBuffer, size_t &rxBufferSize)
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   this->reentrant = true;
   int ret = internalSendReceiveSBD(NULL, txData, txDataSize, rxBuffer, &rxBufferSize);
   this->reentrant = false;
   return ret;
}

// Transmit a text message
int IridiumSBD::sendSBDText(const char *message)
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   this->reentrant = true;
   int ret = internalSendReceiveSBD(message, NULL, 0, NULL, NULL);
   this->reentrant = false;
   return ret;
}

// Transmit a text message and receive reply
int IridiumSBD::sendReceiveSBDText(const char *message, uint8_t *rxBuffer, size_t &rxBufferSize)
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   this->reentrant = true;
   int ret = internalSendReceiveSBD(message, NULL, 0, rxBuffer, &rxBufferSize);
   this->reentrant = false;
   return ret;
}

// High-level wrapper for AT+CSQ
int IridiumSBD::getSignalQuality(int &quality)
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   this->reentrant = true;
   int ret = internalGetSignalQuality(quality);
   this->reentrant = false;
   return ret;
}

// Gracefully put device to lower power mode (if sleep pin provided)
int IridiumSBD::sleep()
{
   if (this->reentrant)
      return ISBD_REENTRANT;

   if (this->sleepPin == NC)
      return ISBD_NO_SLEEP_PIN;

   this->reentrant = true;
   int ret = internalSleep();
   this->reentrant = false;

   if (ret == ISBD_SUCCESS)
      power(false); // power off
   return ret;
}

// Return sleep state
bool IridiumSBD::isAsleep()
{
   return this->asleep;
}

// Return number of pending messages
int IridiumSBD::getWaitingMessageCount()
{
   return this->remainingMessages;
}

// Define capacitor recharge times
void IridiumSBD::setPowerProfile(int profile)
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
void IridiumSBD::adjustATTimeout(int seconds)
{
   this->atTimeout = seconds;
}

// Tweak Send/Receive SBDIX process timeout
void IridiumSBD::adjustSendReceiveTimeout(int seconds)
{
   this->sendReceiveTimeout = seconds;
}

// Diagnostics: attach a serial serial console
void IridiumSBD::attachConsole(Serial &serial)
{
   this->pConsoleSerial = &serial;
}

#if ISBD_DIAGS
// Diagnostics: attach a debug console
void IridiumSBD::attachDiags(Serial &serial)
{
   this->pDiagsSerial = &serial;
}
#endif

void IridiumSBD::setMinimumSignalQuality(int quality)  // a number between 1 and 5, default ISBD_DEFAULT_CSQ_MINIMUM
{
   if (quality >= 1 && quality <= 5)
      this->minimumCSQ = quality;
}

void IridiumSBD::useMSSTMWorkaround(bool useWorkAround) // true to use workaround from Iridium Alert 5/7 
{
   this->useWorkaround = useWorkAround;
}

/* 
Private interface
*/

int IridiumSBD::internalBegin()
{
   dbg(F("Calling internalBegin\r\n"));

   if (!this->asleep)
      return ISBD_ALREADY_AWAKE;

   power(true); // power on
    
   bool modemAlive = false;

   uint32_t startupTime = 500; //ms
    for (uint32_t start = millis(); millis() - start < startupTime;) {
        if (cancelled()) {
            return ISBD_CANCELLED;
        }
    }
   // Turn on modem and wait for a response from "AT" command to begin
   for (unsigned long start = millis(); !modemAlive && millis() - start < 1000UL * ISBD_STARTUP_MAX_TIME;)
   {
      send(F("AT\r"));
      modemAlive = waitForATResponse();
      if (cancelled())
         return ISBD_CANCELLED;
   }

   if (!modemAlive)
   {
      dbg(F("No modem detected.\r\n"));
      return ISBD_NO_MODEM_DETECTED;
   }

   const char* strings[3] = { F("ATE1\r"), F("AT&D0\r"), F("AT&K0\r") };
   for (int i=0; i<3; ++i)
   {
      send(strings[i]); 
      if (!waitForATResponse())
         return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
   }

   dbg(F("InternalBegin: success!\r\n"));
   return ISBD_SUCCESS;
}

int IridiumSBD::internalSendReceiveSBD(const char *txTxtMessage, const uint8_t *txData, size_t txDataSize, uint8_t *rxBuffer, size_t *prxBufferSize)
{
   dbg(F("internalSendReceive\r\n")); // F(s) does casting to __FlashStringHelper*

   if (this->asleep)
      return ISBD_IS_ASLEEP;

   // Binary transmission?
   if (txData && txDataSize)
   {
      send(F("AT+SBDWB="), true, false);
      send(txDataSize);
      send(F("\r"), false);
      if (!waitForATResponse(NULL, 0, NULL, "READY\r\n"))
         return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

      uint16_t checksum = 0;
      for (unsigned int i=0; i<txDataSize; ++i)
      {
         //serial.write(txData[i]);
         // NOTE: We are sending the int as char
         serial.putc(txData[i]);
         checksum += (uint16_t)txData[i];
      }

      console(F("["));
      console((uint16_t)txDataSize); // NOTE: we are casting this to unsigned int
      console(F(" bytes]"));

      dbg(F("Checksum:"));
      dbg(checksum);
      dbg(F("\r\n"));
      //serial.printf("%u", checksum >> 8);
      //serial.printf("%u", checksum & 0xFF);
      serial.putc(checksum >> 8);
      serial.putc(checksum & 0xFF);

      /*
      Command Response:
      0 SBD message successfully written to the 9601-DG.
      1 SBD message write timeout. An insufficient number of bytes were transferred to 9601-DG
      during the transfer period of 60 seconds.
      2 SBD message checksum sent from DTE does not match the checksum calculated at 9601-DG.
      3 SBD message size is not correct. The maximum mobile originated SBD message length is 205
      bytes. The minimum mobile originated SBD message length is 1 byte. 
      */
      // JACKY: checksum response!
      if (!waitForATResponse(NULL, 0, NULL, "0\r\n\r\nOK\r\n"))
      //if (!waitForATResponse(NULL, 0, NULL, "2\r\n\r\nOK\r\n"))
         return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
   }

   else // Text transmission
   {
      send(F("AT+SBDWT="), true, false);
      if (txTxtMessage) // It's ok to have a NULL txtTxtMessage if the transaction is RX only
         send(txTxtMessage);
      send(F("\r"), false);
      if (!waitForATResponse(NULL))
         return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;
   }

   // Long SBDIX loop begins here
   for (unsigned long start = millis(); millis() - start < 1000UL * ISBD_DEFAULT_SENDRECEIVE_TIME;)
   {
      int strength = 0;
      bool okToProceed = true;
      int ret = internalGetSignalQuality(strength);
      if (ret != ISBD_SUCCESS)
         return ret;

      if (useWorkaround && strength >= minimumCSQ)
      {
         okToProceed = false;
         ret = internalMSSTMWorkaround(okToProceed);
         if (ret != ISBD_SUCCESS)
            return ret;
      }

      if (okToProceed && strength >= minimumCSQ)
      {
         uint16_t moCode = 0, moMSN = 0, mtCode = 0, mtMSN = 0, mtLen = 0, mtRemaining = 0;
         ret = doSBDIX(moCode, moMSN, mtCode, mtMSN, mtLen, mtRemaining);
         if (ret != ISBD_SUCCESS)
            return ret;

         dbg(F("SBDIX MO code: "));
         dbg(moCode);
         dbg(F("\r\n"));

         if (moCode <= 4) // successful return!
         {
            dbg(F("SBDIX success!\r\n"));

            this->remainingMessages = mtRemaining;
            if (mtCode == 1 && rxBuffer) // retrieved 1 message
            {
               dbg(F("Incoming message!\r\n"));
               return doSBDRB(rxBuffer, prxBufferSize);
            }

            else
            {
               // No data returned
               if (prxBufferSize) 
                  *prxBufferSize = 0;
            }
            return ISBD_SUCCESS;
         }

         else if (moCode == 12 || moCode == 14 || moCode == 16) // fatal failure: no retry
         {
            dbg(F("SBDIX fatal!\r\n"));
            return ISBD_SBDIX_FATAL_ERROR;
         }

         else // retry      
         {
            dbg(F("Waiting for SBDIX retry...\r\n"));
            if (!smartWait(sbdixInterval))
               return ISBD_CANCELLED;
         }
      }

      else // signal strength == 0
      {
         dbg(F("Waiting for CSQ retry...\r\n"));
         if (!smartWait(csqInterval))
            return ISBD_CANCELLED;
      }
   } // big wait loop

   dbg(F("SBDIX timeout!\r\n"));
   return ISBD_SENDRECEIVE_TIMEOUT;
}

int IridiumSBD::internalGetSignalQuality(int &quality)
{
   if (this->asleep)
      return ISBD_IS_ASLEEP;

   char csqResponseBuf[2];

   send(F("AT+CSQ\r"));
   if (!waitForATResponse(csqResponseBuf, sizeof(csqResponseBuf), "+CSQ:"))
      return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

   if (isdigit(csqResponseBuf[0]))
   {
      quality = atoi(csqResponseBuf);
      return ISBD_SUCCESS;
   }

   return ISBD_PROTOCOL_ERROR;
}

int IridiumSBD::internalMSSTMWorkaround(bool &okToProceed)
{
   /*
   According to Iridium 9602 Product Bulletin of 7 May 2013, to overcome a system erratum:

   "Before attempting any of the following commands: +SBDDET, +SBDREG, +SBDI, +SBDIX, +SBDIXA the field application 
   should issue the AT command –MSSTM to the transceiver and evaluate the response to determine if it is valid or not:

   Valid Response: "---MSSTM: XXXXXXXX" where XXXXXXXX is an eight---digit hexadecimal number.

   Invalid Response: "---MSSTM: no network service"

   If the response is invalid, the field application should wait and recheck system time until a valid response is 
   obtained before proceeding. 

   This will ensure that the Iridium SBD transceiver has received a valid system time before attempting SBD communication. 
   The Iridium SBD transceiver will receive the valid system time from the Iridium network when it has a good link to the 
   satellite. Ensuring that the received signal strength reported in response to AT command +CSQ and +CIER is above 2---3 bars 
   before attempting SBD communication will protect against lockout.
   */
   char msstmResponseBuf[24];

   send(F("AT-MSSTM\r"));
   if (!waitForATResponse(msstmResponseBuf, sizeof(msstmResponseBuf), "-MSSTM: "))
      return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

   // Response buf now contains either an 8-digit number or the string "no network service"
   okToProceed = isdigit(msstmResponseBuf[0]);
   return ISBD_SUCCESS;
}

int IridiumSBD::internalSleep()
{
   if (this->asleep)
      return ISBD_IS_ASLEEP;

   // Best Practices Guide suggests this before shutdown
   send(F("AT*F\r"));

   if (!waitForATResponse())
      return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

   return ISBD_SUCCESS;
}

bool IridiumSBD::smartWait(int seconds)
{
   for (unsigned long start=millis(); millis() - start < 1000UL * seconds;)
      if (cancelled())
         return false;

   return true;
}

// Wait for response from previous AT command.  This process terminates when "terminator" string is seen or upon timeout.
// If "prompt" string is provided (example "+CSQ:"), then all characters following prompt up to the next CRLF are
// stored in response buffer for later parsing by caller.
// Jacky: Example:
//      prompt "+CSQ:", responseSize = 5, response needs to have be 5 bytes long (the last byte of the 5 bytes is set to 0), so only tries
//                                        to match 4 bytes of useful content
//      serial reads: "@#$+CSQ:1234567"
//      response => "1234\0"

   //if (!waitForATResponse(NULL, 0, NULL, "AT+SBDRB\r")) // waits for its own echo
bool IridiumSBD::waitForATResponse(char *response, int responseSize, const char *prompt, const char *terminator)
{
   dbg(F("Waiting for response "));
   dbg(terminator);
   dbg(F("\r\n"));

   if (response)
      memset(response, 0, responseSize);

   int matchPromptPos = 0; // Matches chars in prompt
   int matchTerminatorPos = 0; // Matches chars in terminator
   // 0=prompt not seen, 1=prompt seen, gathering response, 2=response gathered, looking for terminator
   int promptState = prompt ? 0 : 2;
   console(F("<< "));
   for (unsigned long start=millis(); millis() - start < 1000UL * atTimeout;)
   {
      if (cancelled())
         return false;
      while (serial.readable()) {
         char c = (char)serial.getc(); // CAREFUL, we are not sure if getc actually returns values when no more data to be read. We might need to use Serial instead of Stream 
         console(c);
         if (prompt) {
            switch(promptState) {
              case 0: // matching prompt
                 if (c == prompt[matchPromptPos]) {
                    ++matchPromptPos;
                    if (prompt[matchPromptPos] == '\0')
                       promptState = 1;
                 } else {
                    matchPromptPos = c == prompt[0] ? 1 : 0; // try to match prompt, if current char matches, then move on to next char to match
                 }

                 break;
              case 1: // gathering reponse from end of prompt to first \r
                 if (response) {
                    if (c == '\r' || responseSize < 2) { // the reason we check if responseSize < 2 is because we keep decrementing it
                      // when we read in one character, (<2, because the last character is left to store '\0')
                       promptState = 2;
                    } else {
                       *response++ = c; 
                       responseSize--;
                    }
                 }
                 break;
            }
         }

         // If there is no prompt, then just keep trying to match the terminator until either all terminator characters are
         // matched (return true), or no more serial to read (return false)
         if (c == terminator[matchTerminatorPos]) {
            ++matchTerminatorPos;
            if (terminator[matchTerminatorPos] == '\0')
               return true;
         } else {
            matchTerminatorPos = c == terminator[0] ? 1 : 0;
         }
         
      } // while (serial.available() > 0)
   } // timer loop
   return false;
}

bool IridiumSBD::cancelled() // checks if there is a callback function, callback function returns true if want the original process to keep running after the callback is finished
{
   if (ISBDCallback != NULL)
      return !ISBDCallback();

   return false;
}

void IridiumSBD::dbg(const char *msg)
{
#if ISBD_DIAGS
   if (this->pDiagsSerial)
      pDiagsSerial->printf("%s",msg);
#endif
}

void IridiumSBD::dbg(uint16_t n)
{
#if ISBD_DIAGS
   if (this->pDiagsSerial)
      pDiagsSerial->printf("%u",n);
#endif
}

void IridiumSBD::dbg(char c)
{
#if ISBD_DIAGS
   if (this->pDiagsSerial)
      pDiagsSerial->printf("%c",c);
#endif
}

void IridiumSBD::console(const char *msg)
{
   if (this->pConsoleSerial) {
      pConsoleSerial->printf("%s",msg);
  }
}

void IridiumSBD::console(char c)
{
   if (this->pConsoleSerial)
      pConsoleSerial->printf("%c",c);
}

void IridiumSBD::console(uint16_t n)
{
   if (this->pConsoleSerial)
      pConsoleSerial->printf("%u",n);
}

int IridiumSBD::doSBDIX(uint16_t &moCode, uint16_t &moMSN, uint16_t &mtCode, uint16_t &mtMSN, uint16_t &mtLen, uint16_t &mtRemaining)
{
   // xx, xxxxx, xx, xxxxx, xx, xxx
   char sbdixResponseBuf[32];
   send(F("AT+SBDIX\r"));
   if (!waitForATResponse(sbdixResponseBuf, sizeof(sbdixResponseBuf), "+SBDIX: "))
      return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

   uint16_t *values[6] = { &moCode, &moMSN, &mtCode, &mtMSN, &mtLen, &mtRemaining };
   for (int i=0; i<6; ++i)
   {
      char *p = strtok(i == 0 ? sbdixResponseBuf : NULL, ", ");
      if (p == NULL)
         return ISBD_PROTOCOL_ERROR;
      *values[i] = atol(p);
   }
   return ISBD_SUCCESS;
}
/*
bool hasReadTwoBytes(Serial &serial, uint8_t &byte1, uint8_t &byte2, bool &byte1Read) {
  if (serial.readable()) {
    if (!byte1Read) {
      byte1 = (uint8_t)serial.getc();
      byte1Read = true;
    } else {
      byte2 = (uint8_t)serial.getc();
      return true;
    }
  }
  return false;
}
*/
bool hasReadTwoBytes(Serial &serial, uint8_t &byte1, uint8_t &byte2, bool &byte1Read) {
  if (serial.readable()) {
    byte1 = (uint8_t)serial.getc();
    byte2 = (uint8_t)serial.getc();
    byte1Read = true;
    return true;
  }
  return false;
}

uint8_t IridiumSBD::readOneByteFromSerial(unsigned long startTime, 
                                          int *errorCode) {
  // wait for the data to come in
  while (!serial.readable()) {
    if (cancelled()) {
      *errorCode = ISBD_CANCELLED;
      return 0;
    }
    if (millis() - startTime >= 1000UL * atTimeout) {
      dbg(F("Timeout in readOneByteFromSerial...!\r\n"));
      *errorCode = ISBD_SENDRECEIVE_TIMEOUT;
      return 0;
    }
  }
  return (uint8_t)serial.getc();
}

int IridiumSBD::doSBDRB(uint8_t *rxBuffer, size_t *prxBufferSize)
{
   bool rxOverflow = false;

   send(F("AT+SBDRB\r"));
   if (!waitForATResponse(NULL, 0, NULL, "AT+SBDRB\r\n")) // waits for its own echo
      return cancelled() ? ISBD_CANCELLED : ISBD_PROTOCOL_ERROR;

   // Time to read the binary data: size[2], body[size], checksum[2]
   unsigned long start = millis();
   int errorCode = 0;
   // Replace the two reads after the while loop in original Arduino code
   uint8_t szByte1 = readOneByteFromSerial(start, &errorCode);
   if (errorCode != 0) return errorCode;
   uint8_t szByte2 = readOneByteFromSerial(start, &errorCode);
   if (errorCode != 0) return errorCode;

   uint16_t size = 256 * szByte1 + szByte2;
   console(F("[Binary size:"));
   console(size);
   console(F("]"));

   for (int i=0; i<size; ++i) {
     uint8_t c = readOneByteFromSerial(start, &errorCode);
     if (errorCode != 0) return errorCode;

     if (rxBuffer && prxBufferSize) {
        if (*prxBufferSize > 0) {
           *rxBuffer++ = c;
           (*prxBufferSize)--;
        } else {
           rxOverflow = true;
        }
     }
   }
   
   uint8_t cksumByte1 = readOneByteFromSerial(start, &errorCode);
   if (errorCode != 0) return errorCode;
   uint8_t cksumByte2 = readOneByteFromSerial(start, &errorCode);
   if (errorCode != 0) return errorCode;

   uint16_t checksum = 256 * cksumByte1 + cksumByte2; 
   console(F("[csum:"));
   console(checksum);
   console(F("]"));

   // Return actual size of returned buffer
   if (prxBufferSize) 
      *prxBufferSize = (size_t)size;

   return rxOverflow ? ISBD_RX_OVERFLOW : ISBD_SUCCESS;
}

void IridiumSBD::power(bool on)
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
      lastPowerOnTime = millis();
   }

   else
   {
      // Best Practices Guide suggests waiting at least 2 seconds
      // before powering off again
      unsigned long elapsed = millis() - lastPowerOnTime;
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

void IridiumSBD::send(const char *str, bool beginLine, bool endLine)
{
   if (beginLine)
      console(F(">> "));
   console(str);
   if (endLine)
      console(F("\r\n"));
   serial.printf("%s\n",str);
}

void IridiumSBD::send(uint16_t n)
{
   console(n);
   serial.printf("%u",n);
}
