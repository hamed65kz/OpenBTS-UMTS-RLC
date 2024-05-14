
#ifndef TRLCDISCARD_H
#define TRLCDISCARD_H
// There is a discrepancy between the RLC spec 25.322 9.7.3.5, which says:
// "If SDU discard has not been configured for a UM or TM entity SDUs in the transmitter
// shall not be discarded unless the transmission buffer is full."
// and the RRC spec 25.331 10.3.4.25, which says:
// "For UM and TM, only "TimerBasedWithoutExplicitSignaling" is allowed."
#include "../../shareTypes.h"
//#include "../../ASN/o/TransmissionRLC-Discard.h"


struct TransmissionRlcDiscard  // 10.3.4.25
{
    enum SduDiscardMode {
        // See 9.7.3.5.  An AM entity is always configured.
        // For UM and TM, "Not Configured" is one of the options
        // and implies discard without signaling.
        // To represent it here, I am using ...PR_NOTHING, but you cannot
        // put that value in the ASN, rather, the ASN represents by the
        // transmissionRLC_Discard being absent from, eg, struct UL_UM_RLC_Mode.
        NotConfigured ,//= ASN::TransmissionRLC_Discard_PR_NOTHING,
        // Implies MRW after timer.
        TimerBasedWithExplicitSignaling,//, = ASN::TransmissionRLC_Discard_PR_timerBasedExplicit,
        // Section 11.3.4.3 does not document how this
        // mode works, but we wont use it so who cares.
        TimerBasedWithoutExplicitSignaling,// = ASN::TransmissionRLC_Discard_PR_timerBasedNoExplicit,
        // Implies MRW after MaxDAT retransmissions.
        DiscardAfterMaxDatRetransmissions,// = ASN::TransmissionRLC_Discard_PR_maxDAT_Retransmissions,
        // Implies Reset after MaxDAT retransmissions.
        NoDiscard,// = ASN::TransmissionRLC_Discard_PR_noDiscard
    } mSduDiscardMode;
    TransmissionRlcDiscard(): mSduDiscardMode(NotConfigured) {}
    // See 10.3.4.25 for the cases of SduDiscardMode under which these are valid:
    UInt_z mTimerMRW;	// For TimerBasedWithExplicit, DiscardAfterMaxDAT
    UInt_z mMaxMRW;	// For TimerBasedWithExplicit, DiscardAfterMaxDat
    UInt_z mTimerDiscard;	// For TimerBased*, but note that values allowed in enumeration differ.
    UInt_z mMaxDAT;	// For DiscardAfterMaxDat or NoDiscard: Max num transmissions of AMD PDU.
    // For mandatory IE, pass address of IE.
    // For optional IE, pass a 0 ptr and this will return an allocated one,
    // unless the value is NotConfigured, in which case it returns NULL
    // as the value of ASN transmissionRLC_Discard.
//    ASN::TransmissionRLC_Discard *toAsnTRD(ASN::TransmissionRLC_Discard*ptr) {
//        if (mSduDiscardMode == NotConfigured) {
//            // This is a special case in ASN represented by absense of this IE
//            // for RLC-UM or RLC-TM.  But if ptr is specified, it is RLC-AM
//            // and discard mode must always be configured.
//            assert(ptr == NULL);
//            return NULL;
//        }

//        if (ptr == 0) ptr = RN_CALLOC(ASN::TransmissionRLC_Discard);
//        ptr->present = (ASN::TransmissionRLC_Discard_PR) mSduDiscardMode;
//        switch (mSduDiscardMode) {
//        case TimerBasedWithExplicitSignaling:
//            sTimerMRW.cvtAsn(ptr->choice.timerBasedExplicit.timerMRW,mTimerMRW);
//            sTimerDiscard.cvtAsn(ptr->choice.timerBasedExplicit.timerDiscard,mTimerDiscard);
//            sMaxMRW.cvtAsn(ptr->choice.timerBasedExplicit.maxMRW,mMaxMRW);
//            break;
//        case TimerBasedWithoutExplicitSignaling:
//            sNoExplicitDiscard.cvtAsn(ptr->choice.timerBasedNoExplicit, mTimerDiscard);
//            break;
//        case DiscardAfterMaxDatRetransmissions:
//            // Alternate way:
//            //AsnEnum(sMaxDAT,mMaxDAT).toAsn(ptr->choice.maxDAT_Retransmissions.maxDAT);
//            sMaxDAT.cvtAsn(ptr->choice.maxDAT_Retransmissions.maxDAT, mMaxDAT);
//            sTimerMRW.cvtAsn(ptr->choice.maxDAT_Retransmissions.timerMRW,mTimerMRW);
//            sMaxMRW.cvtAsn(ptr->choice.maxDAT_Retransmissions.maxMRW,mMaxMRW);
//            break;
//        case NoDiscard:
//            // Yes, the maxDAT value goes in the ASN variable called noDiscard. barfo
//            sMaxDAT.cvtAsn(ptr->choice.noDiscard,mMaxDAT);
//            break;
//        case NotConfigured:
//            assert(0);	// handled above.
//        }
//        return ptr;
//    }
};

#endif