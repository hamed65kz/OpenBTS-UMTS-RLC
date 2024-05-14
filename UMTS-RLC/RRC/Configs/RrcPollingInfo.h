#ifndef RRCPOLLINGINFO_H
#define RRCPOLLINGINFO_H

#include "../../shareTypes.h"

// 25.331 RRC: 10.3.4.4 Polling Info.
// None or all of the types of polling may be present.
// We will use a value of 0 to indicate the timer is not configured.
struct RrcPollingInfo {	// For RLC
    UInt_z mTimerPollProhibit;	// implemented.
    UInt_z mTimerPoll;				// implemented.	// 9.7.1 paragraph 3. and 9.5
    UInt_z mPollPdu;				// implemented.		// 9.7.1 paragraph 4.
    UInt_z mPollSdu;				// implemented.		// 9.7.1 paragraph 5.
    Bool_z mLastTransmissionPduPoll;	// implemented.		// 9.7.1 paragraph 1.
    Bool_z mLastRetransmissionPduPoll;	// implemented.		// 9.7.1 paragraph 2.
    UInt_z mPollWindow;			// not implemented		// 9.7.1 paragraph 6.
    UInt_z mTimerPollPeriodic;	// implemented.		// 9.7.1 paragraph 7.
//    void toAsnPollingInfo(ASN::PollingInfo *pi)
//    {
//        if (mTimerPollProhibit) {
//            pi->timerPollProhibit = sTimerPollProhibit.allocAsn(mTimerPollProhibit);
//        }
//        if (mTimerPoll) {
//            pi->timerPoll = sTimerPoll.allocAsn(mTimerPoll);
//        }
//        if (mPollPdu) { pi->poll_PDU = sPoll_PDU.allocAsn(mPollPdu); }
//        if (mPollSdu) { pi->poll_SDU = sPoll_SDU.allocAsn(mPollSdu); }
//        pi->lastTransmissionPDU_Poll = mLastTransmissionPduPoll;
//        pi->lastRetransmissionPDU_Poll = mLastRetransmissionPduPoll;
//        if (mPollWindow) { pi->pollWindow = sPollWindow.allocAsn(mPollWindow); }
//        if (mTimerPollPeriodic) {
//            pi->timerPollPeriodic = sTimerPollPeriodic.allocAsn(mTimerPollPeriodic);
//        }
//    }
};
#endif