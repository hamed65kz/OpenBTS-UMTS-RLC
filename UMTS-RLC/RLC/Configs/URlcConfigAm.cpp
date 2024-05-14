#include "URlcConfigAm.h"

URlcConfigAm::URlcConfigAm(URlcInfo &rlcInfo, RrcTfs &dltfs, unsigned dlPduSize) :
        URlcConfigAmUm(URlcModeAm,rlcInfo,dltfs,dlPduSize),
        mInSequenceDeliveryIndication(rlcInfo.mdl.u.AM.mInSequenceDelivery),
        mMaxDAT(rlcInfo.mul.mTransmissionRlcDiscard.mMaxDAT),
        mMaxRST(rlcInfo.mul.u.AM.mMaxRST),
        mConfigured_Tx_Window_Size(rlcInfo.mdl.u.AM.mReceivingWindowSize),
        mConfigured_Rx_Window_Size(rlcInfo.mul.u.AM.mTransmissionWindowSize),
        mTimerRSTValue(rlcInfo.mul.u.AM.mTimerRST),
        mPoll(rlcInfo.mul.u.AM.mPollingInfo),
        mStatusDetectionOfMissingPDU(rlcInfo.mdl.u.AM.mDownlinkRlcStatusInfo.mMissingPduIndicator)
        {}
