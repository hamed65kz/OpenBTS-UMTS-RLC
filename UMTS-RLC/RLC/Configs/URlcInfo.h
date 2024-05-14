#ifndef URLCINFO_H
#define URLCINFO_H

#include "../../shareTypes.h"
#include "TransmissionRlcDiscard.h"
#include "../../RRC/Configs/RrcPollingInfo.h"
#include "../../RLC/Configs/DownlinkRlcStatusInfo.h"
#include "assert.h"

struct URlcInfo 	// 10.3.4.23
{
    struct ul_t {
        enum URlcMode mRlcMode;
        // For UM and TM, only "TimerBasedWithoutExplicitSignaling" is allowed.
        TransmissionRlcDiscard mTransmissionRlcDiscard;	// For AM, UM, TM
        struct ulu_t {	// Its a union, but C++ does not allow them.
            struct ulam_t {
                UInt_z mTransmissionWindowSize;	// For AM only
                UInt_z mTimerRST;	// For AM only
                UInt_z mMaxRST;	// For AM only
                RrcPollingInfo mPollingInfo;	// For AM only
            } AM;
            struct ultm_t {
                // From 25.322 9.2.2.9: If TRUE:
                //	- all pdus carrying segments of sdu shall be sent in one TTI.
                //	- only pdus carrying segments from a single sdu shall be sent one TTI.
                // 	  (So what good is it?  Maybe used with 2ms slotted HS Phy channels?)
                // otherwise:
                //	- PDU size is fixed within a single TTI and equal to SDU size.
                Bool_z mSegmentationIndication;	// For TM only.  TRUE => segment Sdus.
            } TM;
        } u;
        // Note: no Pdu size in uplink.  You can infer the largest pdu size from the Transport Format Set.
        // The inferred largest pdu size implies the LISize as specified in 25.322
    } mul;	// uplink parameters
    struct dl_t {
        enum URlcMode mRlcMode;
        struct dlu_t {	// Its a union, but C++ does not allow them.
            struct dlam_t {
                // Note that 25.332 9.2.2.9 says that flexible size and pdu size are
                // for both AM and UM, because it is post-REL-7.
                // The flexible size option does not exist in our ASN, which is r6,
                // and the option in the 25.331 is marked REL-7.
                Bool_z mRlcPduSizeFlexible;	// REL-7
                UInt_z mDlRlcPduSize;		// AM mode only.  in REL-7, only if not rlcPduSizeFlexible.
                                            // UM mode infers pdu size from TFS.
                // The LI size was added to AM mode in REL-7 and does not exist in our ASN description.
                UInt_z mLengthIndicatorSize;		// REL-7 Length Indicator: 7 or 15.
                                // REL-6: LISize only applicable to UM, AM specifies based on pdusize
                                // NOTE: The UM uplink LISize is different - see above.

                Bool_z mInSequenceDelivery;
                UInt_z mReceivingWindowSize;	// AM only.
                DownlinkRlcStatusInfo	mDownlinkRlcStatusInfo;
            } AM;
            struct dlum_t {
                // ReceptionWindowSize is for UM, ReceivingWindowSize is for AM.
                // Their use is similar but the ranges of possible values of the two are different.
                // NOT USED because we are using an earlier version of ASN.
                UInt_z mDlUmRlcLISize;		// REL-5 Length Indicator: 7 or 15.
                //UInt_z mReceptionWindowSize;	// REL-6 UM only.
            } UM;
            struct dltm_t {
                Bool_z mSegmentationIndication;	// TM only
            } TM;
        } u;
        // These options are outside the union, even though they apply to only one mode.  Whatever.
        Bool_z mOneSidedRlcReEstablishment;	// TM only, not implemented
        //Bool_z mAlternativeEBitInterpretation;	// UM only.
        //Bool_z mUseSpecialValueOfHEField;	// AM only.
    } mdl;	// downlink parameters

    enum URlcMode getUlRlcMode() { return mul.mRlcMode; }
    enum URlcMode getDlRlcMode() { return mdl.mRlcMode; }

    // Default Config setup methods:
    //bool parse_ul;	// Indicates whether we are setting ul or dl options now.

    void ul_RLC_Mode(URlcMode mode) { /*parse_ul=true;*/ mul.mRlcMode = mode; }
    void transmissionRLC_DiscardMode(TransmissionRlcDiscard::SduDiscardMode mode) {
        mul.mTransmissionRlcDiscard.mSduDiscardMode = mode;
    }
    void maxDat(unsigned val) { mul.mTransmissionRlcDiscard.mMaxDAT = val; }
    void transmissionWindowSize(unsigned val) {
        mul.u.AM.mTransmissionWindowSize = val;
    }
    void timerRST(unsigned val) { mul.u.AM.mTimerRST = val; }
    void max_RST(unsigned val) { mul.u.AM.mMaxRST = val; }
    void TimerPoll(unsigned val) { mul.u.AM.mPollingInfo.mTimerPoll = val; }
    void timerPollProhibit(unsigned val) { mul.u.AM.mPollingInfo.mTimerPollProhibit = val; }
    void timerPollPeriodic(unsigned val) { mul.u.AM.mPollingInfo.mTimerPollPeriodic = val; }
    void PollSDU(int val) { mul.u.AM.mPollingInfo.mPollSdu = val; }
    void PollPDU(int val) { mul.u.AM.mPollingInfo.mPollPdu = val; }
    void lastTransmissionPDU_Poll(bool val) { mul.u.AM.mPollingInfo.mLastTransmissionPduPoll = val; }
    void lastRetransmissionPDU_Poll(bool val) { mul.u.AM.mPollingInfo.mLastRetransmissionPduPoll = val; }
    void PollWindow(bool val) { mul.u.AM.mPollingInfo.mPollWindow = val; }

    void timerStatusProhibit(unsigned val) { mdl.u.AM.mDownlinkRlcStatusInfo.mTimerStatusProhibit = val; }
    void timerStatusPeriodic(unsigned val) { mdl.u.AM.mDownlinkRlcStatusInfo.mTimerStatusPeriodic = val; }

    void dl_RLC_Mode(URlcMode mode) { /*parse_ul=false;*/ mdl.mRlcMode = mode; }
    //void dl_RLC_PDU_size(unsigned size) { mdl.u.AM.mDlRlcPduSize = size; }
    void dl_RLC_PDU_size(unsigned size) { assert(0); }	// Let the RLC figure out its own sizes.
    void missingPDU_Indicator(bool val) {
        mdl.u.AM.mDownlinkRlcStatusInfo.mMissingPduIndicator = val;
    }
    // Multiple names for the same thing:
    void dl_UM_RLC_LI_size(unsigned val) { mdl.u.UM.mDlUmRlcLISize = val; }
    void dl_LengthIndicatorSize(unsigned val) { mdl.u.UM.mDlUmRlcLISize = val; }
    void rlc_OneSidedReEst(bool val) { mdl.mOneSidedRlcReEstablishment = val; }

    void ul_segmentationIndication(bool val) { mul.u.TM.mSegmentationIndication = val; }
    void dl_segmentationIndication(bool val) { mdl.u.TM.mSegmentationIndication = val; }
    void inSequenceDelivery(bool val) { mdl.u.AM.mInSequenceDelivery = val; }
    void receivingWindowSize(unsigned val) { mdl.u.AM.mReceivingWindowSize = val; }

//    void toAsnRLC_Info(ASN::RLC_Info *rp) {
//        rp->ul_RLC_Mode = RN_CALLOC(ASN::UL_RLC_Mode);
//        switch (mul.mRlcMode) {
//        case URlcModeAm: {
//            rp->ul_RLC_Mode->present = ASN::UL_RLC_Mode_PR_ul_AM_RLC_Mode;
//            ASN::UL_AM_RLC_Mode *pulam = &rp->ul_RLC_Mode->choice.ul_AM_RLC_Mode;
//            mul.mTransmissionRlcDiscard.toAsnTRD(&pulam->transmissionRLC_Discard);
//            sTransmissionWindowSize.cvtAsn(pulam->transmissionWindowSize,mul.u.AM.mTransmissionWindowSize);
//            sTimerRST.cvtAsn(pulam->timerRST,mul.u.AM.mTimerRST);
//            sMaxRST.cvtAsn(pulam->max_RST,mul.u.AM.mMaxRST);
//            pulam->pollingInfo = RN_CALLOC(ASN::PollingInfo);
//            mul.u.AM.mPollingInfo.toAsnPollingInfo(pulam->pollingInfo);
//            }
//            break;
//        case URlcModeUm: {
//            rp->ul_RLC_Mode->present = ASN::UL_RLC_Mode_PR_ul_UM_RLC_Mode;
//            ASN::UL_UM_RLC_Mode *pulum = &rp->ul_RLC_Mode->choice.ul_UM_RLC_Mode;
//            pulum->transmissionRLC_Discard = mul.mTransmissionRlcDiscard.toAsnTRD(0);
//            }
//            break;
//        case URlcModeTm: {
//            rp->ul_RLC_Mode->present = ASN::UL_RLC_Mode_PR_ul_TM_RLC_Mode;
//            ASN::UL_TM_RLC_Mode *pultm = &rp->ul_RLC_Mode->choice.ul_TM_RLC_Mode;
//            pultm->transmissionRLC_Discard = mul.mTransmissionRlcDiscard.toAsnTRD(0);
//            pultm->segmentationIndication = mul.u.TM.mSegmentationIndication;
//            }
//            break;
//        }

//        // Now the downlink.
//        rp->dl_RLC_Mode = RN_CALLOC(ASN::DL_RLC_Mode);
//        switch (mdl.mRlcMode) {
//        case URlcModeAm: {
//            rp->dl_RLC_Mode->present = ASN::DL_RLC_Mode_PR_dl_AM_RLC_Mode;
//            ASN::DL_AM_RLC_Mode *pdlam = &rp->dl_RLC_Mode->choice.dl_AM_RLC_Mode;
//            pdlam->inSequenceDelivery = mdl.u.AM.mInSequenceDelivery;
//            sReceivingWindowSize.cvtAsn(pdlam->receivingWindowSize,mdl.u.AM.mReceivingWindowSize);
//            mdl.u.AM.mDownlinkRlcStatusInfo.toAsnRSI(&pdlam->dl_RLC_StatusInfo);

//            // Our version ASN does not support:
//            //bool mRlcPduSizeFlexible;	// REL-7
//            //unsigned mDlRlcPduSize;		// in REL-7
//            //unsigned mLengthIndicatorSize;		// REL-7 Length Indicator: 7 or 15.
//            }
//            break;
//        case URlcModeUm:
//            rp->dl_RLC_Mode->present = ASN::DL_RLC_Mode_PR_dl_UM_RLC_Mode;
//            // ASN is completely empty unless you use at least REL-5.
//            //ASN::DL_UM_RLC_Mode *pdlum = &rp->dl_RLC_Mode->choice.dl_UM_RLC_Mode;
//            break;
//        case URlcModeTm: {
//            rp->dl_RLC_Mode->present = ASN::DL_RLC_Mode_PR_dl_TM_RLC_Mode;
//            ASN::DL_TM_RLC_Mode *pdltm = &rp->dl_RLC_Mode->choice.dl_TM_RLC_Mode;
//            pdltm->segmentationIndication = mdl.u.TM.mSegmentationIndication;
//            }
//            break;
//        }
//    }
};

#endif