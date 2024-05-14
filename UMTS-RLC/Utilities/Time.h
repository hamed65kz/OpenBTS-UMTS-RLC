#ifndef TIME_H
#define TIME_H

#include <assert.h>
#include "../shareTypes.h"

const unsigned gHyperframe = 4096;
const unsigned gFrameSlots = 15;


int16_t FNDelta(int16_t v1, int16_t v2);
int FNCompare(int16_t v1, int16_t v2);

/**
        Compare two frame clock values.
        @return 1 if v1>v2, -1 if v1<v2, 0 if v1==v2
*/
int FNCompare(int16_t v1, int16_t v2);

class Time {

    private:

    int16_t mFN;			///< frame number
    int mTN;			///< UMTS radio slot number

    public:

    Time(uint16_t wFN=0, unsigned wTN=0)
        :mFN(wFN % gHyperframe),mTN(wTN)
    { }


    Time slot(unsigned s) const { assert(s<gFrameSlots); return Time(mFN,s); }

    /**@name Accessors. */
    //@{
    int16_t FN() const { return mFN; }
    void FN(uint16_t wFN) { mFN = wFN; }
    unsigned TN() const { return mTN; }
    //void TN(int wTN) { mTN=wTN; }
    //@}

    /**@name Arithmetic. */
    //@{

    Time& operator++()
    {
        mFN = (mFN+1) % gHyperframe;
        return *this;
    }

    Time& decTN(unsigned step=1)
    {
        mTN -= step;
        if (mTN<0) {
            mTN+=gFrameSlots;
            mFN--;
            if (mFN<0) mFN+=gHyperframe;
        }
        return *this;
    }

    Time& incTN(unsigned step=1)
    {
        mTN += step;
        if (mTN>=(int)gFrameSlots) {
            mTN-=gFrameSlots;
            mFN = (mFN+1) % gHyperframe;
        }
        return *this;
    }

    Time& operator+=(int step)
    {
        // Remember the step might be negative.
        mFN += step;
            if (mFN<0) mFN+=gHyperframe;
                mFN = mFN % gHyperframe;
        return *this;
    }

    Time operator-(int step) const
        { return operator+(-step); }

    Time operator+(int step) const
    {
        Time newVal = *this;
        newVal += step;
        return newVal;
    }

    Time operator+(const Time& other) const
    {
        unsigned sTN = mTN+other.mTN;
        unsigned newTN = sTN % gFrameSlots;
        int16_t newFN = mFN+other.mFN + (sTN/gFrameSlots);
        newFN = newFN % gHyperframe;
        return Time(newFN,newTN);
    }

    int operator-(const Time& other) const
        { return FNDelta(mFN,other.mFN); }

    //@}


    /**@name Comparisons. */
    //@{

    bool operator<(const Time& other) const
    {
        if (mFN==other.mFN) return (mTN<other.mTN);
        return FNCompare(mFN,other.mFN) < 0;
    }

    bool operator>(const Time& other) const
    {
        if (mFN==other.mFN) return (mTN>other.mTN);
        return FNCompare(mFN,other.mFN) > 0;
    }

    bool operator<=(const Time& other) const
    {
        if (mFN==other.mFN) return (mTN<=other.mTN);
        return FNCompare(mFN,other.mFN) <=0;
    }

    bool operator>=(const Time& other) const
    {
        if (mFN==other.mFN) return (mTN>=other.mTN);
        return FNCompare(mFN,other.mFN)>=0;
    }

    bool operator==(const Time& other) const
        { return (mFN == other.mFN) && (mTN==other.mTN); }

    //@}

};

std::ostream& operator<<(std::ostream& os, const Time& ts);


#endif // TIME_H
