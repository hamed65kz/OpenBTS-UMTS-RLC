#include "URlcBase.h"
#include "../Ue/UEInfo.h"

URlcBase::URlcBase(URlcMode wRlcMode, UEInfo* wUep, RBInfo* wRbp) :
	mRlcState(RLC_RUN), mRlcMode(wRlcMode), mUep(wUep), mrbid(wRbp->mRbId)
{
	switch (wRlcMode) {
	case URlcModeTm: mSNS = 0; break;
	case URlcModeUm: mSNS = UmSNS; break;
	case URlcModeAm: mSNS = AmSNS; break;
	default: assert(0);
	}
}

int URlcBase::deltaSN(URlcSN sn1, URlcSN sn2)
{
    int ws = mSNS/2;
    int delta = (int)sn1 - (int)sn2;
    //assert(!(delta >= ws));
    if (delta < - ws) delta += mSNS;      // modulo the sequence space
    if (delta > ws) delta -= mSNS;
    return delta;
}

// Warning: the numbers can be negative.
URlcSN URlcBase::addSN(URlcSN sn1, URlcSN sn2)
{
    return ((unsigned)((int)sn1 + (int)sn2)) % (unsigned) mSNS;
}

URlcSN URlcBase::minSN(URlcSN sn1, URlcSN sn2)
{
    return (deltaSN(sn1,sn2) <= 0) ? sn1 : sn2;
}

URlcSN URlcBase::maxSN(URlcSN sn1, URlcSN sn2)
{
    return (deltaSN(sn1,sn2) >= 0) ? sn1 : sn2;
}

void URlcBase::incSN(URlcSN &psn)
{
    psn = addSN(psn,1);
}
