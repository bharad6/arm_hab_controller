/**
 * @author Shane Leonard
 * @date   2015/11/18
 *
 * Implementation of the watchdog class for the STM32 NUCLEO L152RE.
 *
 * Citations of the form [Section 1.2] refer to the appropriate section of the
 * STM32L152xx reference manual, found at:
 *
 * http://www.st.com/web/en/resource/technical/document/reference_manual/CD00240193.pdf
 */

#include "Watchdog.h"

#include "mbed.h"

/* Log2 Function. Can use an optimized version if compiling with GCC. */
#if defined(__GNUC__)
static unsigned log2(unsigned val)
{
    return (val == 0) ? 0 : 32 - __builtin_clz(val);
}
#else
static unsigned log2(unsigned val)
{
    unsigned r = 0;

    while (val >>= 1) r++;

    return r;
}
#endif

Watchdog::Watchdog()
{
    /*
     * Read the reset status register on startup to determine whether the
     * watchdog was the cause of the previous reset.
     */
    reset_state = (bool)(RCC->CSR & (1 << 29));
    if (reset_state)
    {
        /* Clear the reset flag. */
        RCC->CSR |= (1 << 24);
    }
}

/* 
 * Start the watchdog, setting it to timeout after a given duration of time.
 *
 * @param interval Timeout interval, in seconds.
 *
 * @note On the STM32, the watchdog timer interval is limited to 26.2 seconds.
 */
void Watchdog::Start(float interval)
{
    /* The newer STM Nucleo boards contain an on-board 32.768 kHz crystal. */
    const int lsi_freq = 32768;
    unsigned ticks = lsi_freq * interval;

    /*
     * Calculate the correct prescaler and reload register value based off
     * of the given interval.
     */
    int scale = 1 + log2(ticks / 4096);
    int rlr = ticks / (1 << scale);

    /* Anything above the maximum timeout will clamp to ~26.2 seconds. */
    if (scale > 8)
    {
        scale = 8;
        rlr = 0xFFF;
    }

    /* Disable write protection for PR, RLR [Section 21.3.2].  */
    IWDG->KR = 0x5555;

    /* Initialize the prescaler value.  */
    IWDG->PR = scale - 2;

    /* Initialize the reload register.  */
    IWDG->RLR = rlr;

    /* Reload the watchdog [Section 21.3]. */
    IWDG->KR = 0xAAAA;

    /* Start the watchdog [Section 21.3]. */
    IWDG->KR = 0xCCCC;
}

/* Temporarily quiet the watchdog by petting (kicking, feeding, etc) it. */
void Watchdog::Pet()
{
    /* Writing 0xAAAA to the key register restarts the timer [Section 21.3]. */
    IWDG->KR = 0xAAAA;
}

/* Returns true if the previous reset was caused by the watchdog. */
bool Watchdog::WatchdogCausedPreviousReset()
{
    return reset_state;
}
