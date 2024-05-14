#include "Z100Timer.h"


bool Z100Timer::expired() const
{
    assert(mLimitTime!=0);
    // A non-active timer does not expire.
    if (!mActive) return false;
    return mEndTime.passed();
}

void Z100Timer::set()
{
    assert(mLimitTime!=0);
    mEndTime = Timeval(mLimitTime);
    mActive=true;
}

void Z100Timer::expire()
{
    mEndTime = Timeval(0);
    mActive=true;
}


void Z100Timer::set(long wLimitTime)
{
    mLimitTime = wLimitTime;
    set();
}


long Z100Timer::remaining() const
{
    if (!mActive) return 0;
    long rem = mEndTime.remaining();
    if (rem<0) rem=0;
    return rem;
}

void Z100Timer::wait() const
{
    while (!expired()) msleep(remaining());
}
