// This is the config as used by the URLC classes.
// The URLC Config is specified primarily in 25.322 8.2 Primitive Parameters,
// paragraph 7 for AM_parameters, paragraph 11 for UM_parameters,
// and paragraph 12 for TM_parameters.
// Parameters that are implemented in the common URlcTransAmUm or URlcRecvAmUm classes
// are defined in the ConfigCommon class even if they only apply to one mode.
#ifndef URLCCONFIGAMUM_H
#define URLCCONFIGAMUM_H

#include "URlcInfo.h"
#include "TransmissionRlcDiscard.h"
#include "../../RRC/Configs/URRCTrCh.h"

//#include "../../shareTypes.h"

struct URlcConfigAmUm
{
    // 25.331 RRC 10.3.4.25 Transmission RLC Discard IE
    ///<@name RLC SDU Discard Mode 25.322 9.7.3
    // For TM and UM only TimerBasedWithoutExplicitSignaling, which is irrelevant
    // to us because our TM and UM implementation just ignores this field.
    // For AM the only AM mode we support is NoDiscard.
    // So this whole DiscardMode is currently irrelevant.
    //TransmissionRlcDiscard::SduDiscardMode mDiscardMode;
    TransmissionRlcDiscard mRlcDiscard;
    // For Am, the pdu size is specifed in the RlcInfo IE, and Um uses any of the
    // pdu sizes specified in the TransferFormatSet.
    // In REL-7, they added flexible pdu size to AM, which I think just makes it work like UM.
    // However, we are not going to support multiple PduSizes, so there is just one,
    // and here it is:
    // We dont use 'flexible size', so it is not even mentioned here.
    unsigned mDlPduSizeBytes;
    // The uplink and downlink LI size can be configured separately, several different ways.
    // AM mode is determined from the pduSize, or in REL-7, if flexiblePduSize is selected,
    // it is specified in the RlcInfo 25.331 10.3.4.23
    // For UM mode the downlink LISize is in RlcInfo, and the uplink LIsize is determined
    // indirectly from the largest PDU in the TransferFormatSet using rules
    // in 25.322 (RLC) 9.2.2.8 (Length Indicator)
    unsigned mUlLiSizeBytes, mUlLiSizeBits;
    unsigned mDlLiSizeBytes, mDlLiSizeBits;	// 9.2.2.8
    Bool_z mIsSharedRlc;	// This is the special RLC to run CCCH.

    URlcConfigAmUm(URlcMode rlcMode, URlcInfo &rlcInfo, RrcTfs &dltfs, unsigned dlPduSize);

    // These are UM-only parameters, but implemented in the URlcTransAmUm class, so they go here.
    struct {
        // This is post-REL-6 and is not in our ASN spec.
        // For AM mode, it should always be FALSE.
        Bool_z mSN_Delivery;	// This config option causes SN indications to be passed to higher layers
                        // in UM mode, and further indicates not to concatenate SDUs in a PDU.
        // Must be FALSE for AM mode.
        Bool_z mAltEBitInterpretation;	// 9.2.2.5
    } UM;

    static const unsigned mMaxSduSize = 1502;
};
#endif