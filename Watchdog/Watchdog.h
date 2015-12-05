/**
 * @author Shane Leonard
 * @date   2015/11/18
 *
 * Independent watchdog API for the STM32L1 series.
 *
 * @note Based on Arash Salarian's watchdog implementation.
 * https://developer.mbed.org/users/salarian/code/Watchdog/
 */

#include "mbed.h"

/**
 * Independent watchdog timer, which will reset the microcontroller if it isn't
 * pet ("kicked", "fed", ...) often enough.
 *
 * @note On the STM32L1 series, there is an independent watchdog timer and a
 * window watchdog timer. This class only implements the indepedent watchdog
 * timer, and so should be used for applications which don't need a reset to
 * occur within a tightly-defined time interval.
 */
class Watchdog
{
public:
    Watchdog();

    /**
     * Configure the watchdog to start counting down, at the specified time
     * interval.
     *
     * @param interval Watchdog countdown time, in seconds. 
     *
     * @note On the STM32, the maximum timer interval is limited to 26.2 seconds.
     */
    void Start(float interval);

    /**
     * Pet ("feed", "kick") the watchdog to reset its countdown timer.
     */
    void Pet();

    /**
     * Returns true if the microcontroller was reset by the watchdog.
     */
    bool WatchdogCausedPreviousReset();

private:
    bool reset_state;
};

