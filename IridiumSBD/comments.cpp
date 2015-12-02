/* This is a comment in Arduino Iridium website who mentioned
that there is potential bug in original arduino version's iridium 
library when reading data*/
for (int i=0; size > i; ++i)
{
  // wait for the data to come in
  while (!stream.available())
  {
    if (cancelled())
      return ISBD_CANCELLED;
    if (millis() – start >= 1000UL * atTimeout)
      return ISBD_SENDRECEIVE_TIMEOUT;
  }
  uint8_t c = stream.read();
  if (rxBuffer && prxBufferSize) {
    if (*prxBufferSize > 0)
    {
      *rxBuffer++ = c;
      (*prxBufferSize)--;
    }
    else
    {
      rxOverflow = true;
    }
  }
}
