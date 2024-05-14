#include "URlcPdu.h"
#include "../Utilities/Logger.h"

URlcPdu::URlcPdu(unsigned wSize, URlcBase* wOwner, string wDescr)
	: URlcBasePdu(wSize, wDescr), mOwner(wOwner),
	mPaddingStart(0), mPaddingLILocation(0),
	mVTDAT(0), mNacked(0), mNext(0)
{}

URlcPdu::URlcPdu(const BitVector& bits, URlcBase* wOwner, string wDescr)
	: URlcBasePdu(bits, wDescr), mOwner(wOwner),
	mPaddingStart(0), mPaddingLILocation(0),
	mVTDAT(0), mNacked(0), mNext(0)
{}
URlcPdu::URlcPdu(URlcPdu* other)
	: URlcBasePdu(*other, other->mDescr), mOwner(other->mOwner),
	mPaddingStart(other->mPaddingStart),
	mPaddingLILocation(other->mPaddingLILocation),
	mVTDAT(other->mVTDAT), mNacked(other->mNacked), mNext(0)
{}
void URlcPdu::appendLIandE(unsigned licnt, unsigned E, unsigned lisize)
{
        if (lisize == 1) {
                appendByte((licnt<<1) | E);
        } else {
                appendUInt16((licnt<<1) | E);
        }
}


void URlcPdu::text(std::ostream &os) const
{
    os <<" URlcPdu(";
    switch (rlcMode()) {
    case URlcModeTm:
        os <<"TM" <<LOGVAR2("size",size());
        break;
    case URlcModeUm:
        os <<"UM" <<LOGVAR2("sn",getSN());
        os <<LOGVAR2("payloadSize",getPayloadSize());
        os <<LOGVAR2("E",getEIndicator());
        break;
    case URlcModeAm:
        if (getAmDC()) {
            os <<"AM" <<LOGVAR2("sn",getSN());
            os <<LOGVAR2("payloadSize",getPayloadSize());
            os <<LOGVAR2("E",getEIndicator());
        } else {
            switch ((int)getField(1,3)) {
            case 0: os <<"AM control type=status sufi="<<(int)getField(4,4); break;
            case 1: os <<"AM control type=reset"; break;
            case 2: os <<"AM control type=reset ack"; break;
            default: os <<"AM control type=unrecognized"; break;
            }
        }
    }
    os <<"\"" <<mDescr <<"\"";
    os <<")";
}

//URlcPdu::URlcPdu(ByteVector *other, string wDescr)	// Used to manufacture URlcPdu from URlcDownSdu for RLC-TM.
//	: ByteVector(*other), mOwner(0), mDescr(wDescr),
//	mPaddingStart(0),
//	mPaddingLILocation(0),
//	mVTDAT(0), mNacked(0), mNext(0)
//	{}
