#ifndef URLCCONFIGAM_H
#define URLCCONFIGAM_H
#include "URlcConfigAmUm.h"

// 25.322 8.2 paragraph 7: Primitive parameters for AM.
// These are the primitive paramters as used by the RLC code.
// We create this struct when we create an RLC entity.
// The parameter names in the RLC spec do not match 25.321 RRC, and their configured values
// come from several places, including 10.4.3.23 RlcInfo, or may be computed from the
// TransportSet, for example the Max RLC pdu size.
// Therefore we will copy the subset of parameters that we actually use from the RRC IEs
// into this structure when we create an RLC entity.
struct URlcConfigAm : public /*virtual*/ URlcConfigAmUm	// AM only config stuff.
{
    bool mInSequenceDeliveryIndication;
    ///<@name RLC Protocol Parameters, 3GPP 25.322 9.6
    unsigned mMaxDAT;
    //unsigned mPoll_Window;
    unsigned mMaxRST;	// Max-1 number of RESET PDUs, upper limit for VTRST
    unsigned mConfigured_Tx_Window_Size;
    unsigned mConfigured_Rx_Window_Size;
    //unsigned mMaxMRW;		// unimplemented
    ///<@name RLC Timer values 25.322 9.5
    // Timer_Poll, Timer_Poll_Prohibit, Timer_Poll_Periodic - see PollingInfo
    // mTimer_Discard;	Not implemented yet, but we probably want it.
    // Timer_Status_Prohibit, Timer_Status_Periodic  - see RLC Status Triggers.
    unsigned mTimerRSTValue;	// 11.4.2
    // Timer_MRW not implemented.
    ///<@name RLC Polling Triggers 25.322 9.7.1
    RrcPollingInfo mPoll;
    ///<@name RLC Status Triggers 25.322 9.7.2
    bool mStatusDetectionOfMissingPDU;

    // TODO: mTimer_Status_Periodic
    ///<@name RLC Periodical Status Blocking 25.322 9.7.2
    // TODO: mTimer_Status_Prohibit TODO
    ///<@name RLC Minimum WSN 25.322 9.2.2.11.3
    // WONT DO: unsigned mMinWSN;
    ///<@name RLC Send MRW.
    // WONT DO: unsigned mSendMRW;

    //unsigned mHFN;

    URlcConfigAm(URlcInfo &rlcInfo, RrcTfs &dltfs, unsigned dlPduSize);
};

#endif
