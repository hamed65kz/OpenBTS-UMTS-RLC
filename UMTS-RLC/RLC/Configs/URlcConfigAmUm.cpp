#include "URlcConfigAmUm.h"

URlcConfigAmUm::URlcConfigAmUm(URlcMode rlcMode, URlcInfo &rlcInfo, RrcTfs &dltfs, unsigned dlPduSize):
    mRlcDiscard(rlcInfo.mul.mTransmissionRlcDiscard)
{
    if (rlcMode == URlcModeAm) {
        //	We dont need this, and I dont think it is even defined in our REL of the ASN description.
        //if (mRlcPduSizeFlexible) {
        //	mDlPduSizeBytes = dltfs.getLargestPduSize();
        //	mUlLiSize = mDlLiSizeBits = rlcInfo.mdl.u.AM.mLengthIndicatorSize;
        //} else
        {
            // In the specs for the default RB configurations, the pdusize is sometimes specified
            // in the rlcInfo options, having been calculated by hand, but sometimes not.
            // We are going to figure out the RLC size directly
            // for AM and UM mode from the TB size and Mac header bits and pass it in
            // through the configuration constructors.
            //mDlPduSizeBytes = rlcInfo.mdl.u.AM.mDlRlcPduSize;
            mDlPduSizeBytes = dlPduSize;
            mUlLiSizeBits = mDlLiSizeBits = (mDlPduSizeBytes <= 126) ? 7 : 15;
            //mUlLiSizeBits = 7;
        }
    } else {
        mDlPduSizeBytes = dlPduSize;
        // TODO: Our version of ASN does not transmit mDlUmRlcLISize, and no one ever set it.
        // TODO: The RLC size should be determined from the TFS.
        //mDlLiSizeBits = rlcInfo.mdl.u.UM.mDlUmRlcLISize;
        // 8.6.4.9 of 25.331 says that Downlink UM LI is 7 bits unless explicitly indicated, regardless of PDU size
        mDlLiSizeBits = 7; //(mDlPduSizeBytes <= 125) ? 7 : 15;
        mUlLiSizeBits = (mDlPduSizeBytes <= 125) ? 7 : 15;
        //mUlLiSizeBits = 7;
    }
    switch (mDlLiSizeBits) {
        case 7: mDlLiSizeBytes = 1; break;
        case 15: mDlLiSizeBytes = 2; break;
        default: assert(0);
    }
    switch (mUlLiSizeBits) {
        case 7: mUlLiSizeBytes = 1; break;
        case 15: mUlLiSizeBytes = 2; break;
        default: assert(0);
    }
    //printf("mode=%s pdusize=%d dllibits=%d dllibytes=%d\n", rlcMode == URlcModeAm?"AM":"UM",
        //mDlPduSizeBytes,mDlLiSizeBits,mDlLiSizeBytes);
}
