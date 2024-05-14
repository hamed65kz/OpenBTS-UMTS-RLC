#ifndef RBINFO_H
#define RBINFO_H

#include "../RRC/Configs/RrcDefs.h"
#include "../RLC/Configs/URlcInfo.h"


#define gsMaxRB  16	// SRB0 - SRB4 + data rb 5..15

// Note: In the RLC Protocol Parameters, these have different names as follows:
// Configured_Tx_Window_Size == uplink.TransmissionWindowSize
// Configured_Rx_Window_Size == downlink.ReceivingWindowSize
// The ReceptionWindowSize is for UM mode and the ReceivingWindowSize
// SN_Delivery is found where?

    // The "Largest UL UM PDU size" is one of the primitive params
    // as per 25.322 8.2 paragraph 11, but it is not specified in this structure,
    // it is inferred from the RB Mapping Info, which is the next struct above this one.
    // I am adding the value here for use by the RLC, but this is not transmitted in ASN.
    //unsigned cLargestUlUmPduSize;

// 3GPP 24.331 10.3.4.2 PDCP Info.
// It is most likely included in 10.3.4.18 RB Information to Reconfigure,
// which is most likely included in 10.2.33 Radio Bearer Setup Message.
// There does not need to be anything in this structure if we dont support compression.
struct PdcpInfo
{
    static const bool mSrnsSupport = false;
    static const bool mPdcpPduHeader = false;	// absent;
    static const unsigned mAlgorithmType = 0;	// No compression.
};



// There are several nearly identical IEs:
// 10.3.4.18: RB information to reconfigure, used in Cell Update Confirm.
// 10.3.4.20: RB information to Set up is used in:
//		10.3.4.10 RAB Information for setup, which is used in 10.2.33: Radio Bearer Setup
//		in 10.3.4.7 Predefined RB configuration, we wont use.
// 		SRNS Relocation Info, we wont use.
// 3-27-2012: Throw away RBMappingInfo and use a default mapping based on the TrCh.
struct RBInfo : public URlcInfo, public PdcpInfo, public virtual RrcDefs
{
    // from 10.3.4.18

    // Unneeded: PDCPSNInfo; it is optional; used for lossless SRNS relocation.

    int mRbId;	// logical channel id 10.3.4.16, specified as 1..32 in the spec,
            // but I use the same struct for SRB0, in which case id==0.
    // For 10.3.4.20 only, the Rlc info can come from some other RB.
    enum ChoiceRLCInfoType { eRlcInfo, eSameAsRB } mChoiceRlcInfoType;
    // If mChoiceRlcInfo == RlcInfo, use the URlcInfo above; otherwise setup from this RB:
    //struct {
    //	unsigned mRbId2;
    //} SameAsRB;
    enum RBStopContinue { Stop, Continue } mRBStopContinue;	// unused yet.

    // We need to know which RBs are RABs for the Radio Bearer Setup Message.
    // Only applies to RB-id >= 5, which we assume is a RAB (or sub-rab-flow.)
    // We dont allow mapping SRBs with id >= 5.
    // If it is CS domain, that rbid 5,6,7 constituate a RAB for the AMR codec.
    // You dont have to set mPsCsDomain for SRBs (which have rbid <= 4.)
    CNDomainId mPsCsDomain; // CS or PS domain.
    bool isCsDomain() {
        assert(mRbId >= 5); // Only call this function on RB-ids >= 5.
        assert(mPsCsDomain == CSDomain || mPsCsDomain == PSDomain);
        return mPsCsDomain == CSDomain;
    }

    // Normally the RBMappingInfo specifies the mapping of rb->trch.
    // We do that statically, so the RBMappingInfo appears in two places:
    // The RBInfo specifies the trch (here) and the TrChInfo specifies
    // the multiplexing option, and if not multiplexed, the single rbid
    // to be sent on that channel.
    TrChId mTrChAssigned;	// The trch id (0-based) that carries this logical channel.
    void setTransportChannelIdentity(unsigned tcid1based) {	// incoming is 1-based.
        assert(tcid1based >= 1 && tcid1based < 32);
        mTrChAssigned = tcid1based-1;
    }

    RBInfo(): mRbId(-1), mPsCsDomain(UnconfiguredDomain), mTrChAssigned(0) {}
    bool valid() { return mRbId >= 0; }

    // Default Config Setup Functions:
    // If it is an srb, the domain need not be specified.
    void rb_Identity(unsigned rbid, CNDomainId domain=UnconfiguredDomain) {
        assert(rbid <= 4 || domain != UnconfiguredDomain);
        mRbId = rbid;
        mPsCsDomain = domain;
    }

    // This is a generic RLC-AM config, from defaultConfig0CFRb below.
    void defaultConfigSrbRlcAm();
    void defaultConfigRlcAmPs();	// One for packet-switched RLCs.

    // The first one applies to us.
    // Some are from 34.108: Common Test Environments for UE testing, list
    // in sec 6.10.2, with the RAB and RBs defined immediately before.
    // Some are from 25.993: Typical Examples of RABs and RBS supported by UTRA.
    // 25.331 RRC 13.6: RB information parameters for SRB0
    void defaultConfigSRB0();

    // 25.331 RRC 13.8: Default configuration 0 when using CELL_FACH.
    // This is the RB part; the TrCh part goes in TrChConfig
    void defaultConfig0CFRb(unsigned rbn);

    // 25.331 RRC 13.7: Other Default configurations
    void defaultConfig3Rb(unsigned rbn);
};

#endif




