#ifndef Z100TIMER_H
#define Z100TIMER_H

#include "Timeval.h"
#include "assert.h"

/**
    CCITT Z.100 activity timer, as described in GSM 04.06 5.1.
    All times are in milliseconds.
*/
class Z100Timer {

    private:

    Timeval mEndTime;		///< the time at which this timer will expire
    long mLimitTime;		///< timeout in milliseconds
    bool mActive;			///< true if timer is active

    public:

    /** Create a timer with a given timeout in milliseconds. */
    Z100Timer(long wLimitTime)
        :mLimitTime(wLimitTime),
        mActive(false)
    {}

    /** Blank constructor; if you use this object, it will assert. */
    Z100Timer():mLimitTime(0),mActive(false) {}

    /** True if the timer is active and expired. */
    bool expired() const;

    /** Force the timer into an expired state. */
    void expire();

    /** Start or restart the timer. */
    void set();

    /** Start or restart the timer, possibly specifying a new limit. */
    void set(long wLimitTime);

    // Just configure, dont start.  Value may be 0, but dont start the timer with that value.
    void configure(long wLimitTime) { mLimitTime = wLimitTime; }

    /** Stop the timer. */
    void reset() { assert(mLimitTime!=0); mActive = false; }

    /** Returns true if the timer is active. */
    bool active() const { return mActive; }

    /** Returns true if the timer has been configured with a non-zero limit. */
    bool configured() const { return mLimitTime != 0; }

    /**
        Remaining time until expiration, in milliseconds.
        Returns zero if the timer has expired.
    */
    long remaining() const;

    /**
        Block until the timer expires.
        Returns immediately if the timer is not running.
    */
    void wait() const;
};

#endif
