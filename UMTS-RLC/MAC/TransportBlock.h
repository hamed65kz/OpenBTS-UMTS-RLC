#ifndef TRANSPORTBLOCK_H
#define TRANSPORTBLOCK_H
#include "../Utilities/BitVector.h"
#include "../Utilities/Time.h"


/**
    The transport block is passed on the simple L1/MAC interface,
    which supports only one TrCh per L1 connection per RadioFrame.
    This class is over-ridden in the MAC engine to aid in TransportBlock creation,
    but only the information here is needed to be passed on the L1/MAC interface.
*/
class TransportBlock : public BitVector {

    Time mTime;	///< the time when this block was receive or is to be transmitted.
    public:
    bool mScheduled;	///< if false, ignore mTime
    // (pat) No, the tfci does not go in the TB, it goes in the TBS.
    //unsigned mTfci;		///< the TFCI [Transport Format Combination Index] assigned by MAC.
    std::string mDescr;	///< Optional description of what is in it.


    TransportBlock(const BitVector& bits)
        :BitVector(bits),mScheduled(false)//,mTfci(0)
    { }

    TransportBlock(const BitVector& bits, const Time& wTime)
        :BitVector(bits),
        mTime(wTime),mScheduled(true)//,mTfci(0)
    { }

    TransportBlock(size_t sz)
        :BitVector(sz),mScheduled(false)//,mTfci(0)
    { }

    TransportBlock(size_t sz, const Time& wTime)
        :BitVector(sz),
        mTime(wTime),mScheduled(true)//,mTfci(0)
    { }

    TransportBlock(const TransportBlock& block, const Time& wTime)
        :BitVector(block),
        mTime(wTime),mScheduled(true)//,mTfci(0)
    { }

    void setSchedule(unsigned framenum) { mTime = Time(framenum); mScheduled = true; }

    bool scheduled() const { return mScheduled;}

    const Time& time() const { assert(mScheduled); return mTime; }
    void time(const Time& wTime) { mTime = wTime; mScheduled=true; }

    //void encodeParity(Parity& parity, BitVector& bBits) const;

    friend std::ostream& operator<<(std::ostream& os, const TransportBlock&);
    friend std::ostream& operator<<(std::ostream& os, const TransportBlock*);
    void text(std::ostream &os) const;
};

std::ostream& operator<<(std::ostream& os, const TransportBlock&);
std::ostream& operator<<(std::ostream& os, const TransportBlock*);

#endif // TRANSPORTBLOCK_H
