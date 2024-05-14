#include "RBInfo.h"


// 25.331 13.6: RB information parameters for SRB0
void RBInfo::defaultConfigSRB0()
{
    rb_Identity(0);	// Not in the spec, pat added.
    ul_RLC_Mode(URlcModeTm);
    ul_segmentationIndication(false);
    dl_RLC_Mode(URlcModeUm);	// Thats right: TM up but UM down.
    dl_LengthIndicatorSize(7);
#if USE_RBMAPPINGINFO	// Unneeded now.
    // RB Mapping Info
    UL_LogicalChannelMappings(1);
    MappingOption(1);
    ul_TransportChannelType(TrChRACHType);
    // Note: RLC size uses IE "Additional Transport Format Information for CCCH"
    mac_LogicalChannelPriority(1);
    // This is not in the 13.6 spec, but we have to set it to the one and only TrCh:
    ul_transportChannelIdentity(1);
    dl_transportChannelIdentity(1);
#endif
}

// Default config for SRBs using RLC-AM.
// Came from 3GPP 25.331 13.8 for SRB1, SRB2, SRB3
// Once we program the SRBs we want to leave their RLC setup the same, except for
// RLC-size, regardless of transitions between CELL_FACH and CELL_DCH,
// because when we setup the new RLCs for a UE state change in ueConnectRlc(),
// we do it by simply re-using the same RLC, which implies we cannot change
// any of its other programming.  As a corollary, that means we can affect
// the UE state change RLC programming by sending only the RBMappingInfo.
void RBInfo::defaultConfigSrbRlcAm()
{
    // This is designed for SRBs, so it forces each SDU completely through
    // using a poll before proceeding; note that PollSDU == 1.
    // The TimerPoll is required so that if the PDU that has the poll
    // bit is lost, it will be retransmitted after the timeout.
    ul_RLC_Mode(URlcModeAm);
    transmissionRLC_DiscardMode(TransmissionRlcDiscard::NoDiscard);
    maxDat(40);
    transmissionWindowSize(64);
    timerRST(200);
    max_RST(1);			// Interesting that only one reset attempt allowed.
    // max_RST must be 1 for SRB2, or UE behavior is unspecified
    TimerPoll(200);	// 1 sec seems pretty long.
    PollSDU(1);			// Poll after every SDU.
    lastTransmissionPDU_Poll(false);
    lastRetransmissionPDU_Poll(true);
    PollWindow(99);
    dl_RLC_Mode(URlcModeAm);
    // (pat) They specify the PDU size per-RB in the spec, but we are going to let the
    // RLC figure it out, which is much safer.
    //dl_RLC_PDU_size(128);	// actual size 144
    inSequenceDelivery(true);
    receivingWindowSize(64);
    missingPDU_Indicator(true);
    // rlc_OneSidedReEst(false);
}

// Default config for a packet-switched data channel.
// Pat just made this up from scratch.
void RBInfo::defaultConfigRlcAmPs()
{
    defaultConfigSrbRlcAm();	// Start with this config.
    transmissionWindowSize(2047);	// Lots more data, so increase the window size.
    max_RST(8);				// Allow more failures
    // The following should vary based on chosen TTI.
    lastTransmissionPDU_Poll(false);	// Poll on the last PDU.
    lastRetransmissionPDU_Poll(true);	// or last retransmitted PDU.
    TimerPoll(1000);				// Reduce poll timeout - 100ms = every 10 PDUs.
    PollSDU(0);				// Dont do this.
    PollPDU(0);			// Do this instead.
    inSequenceDelivery(false);
    receivingWindowSize(2047);
    PollWindow(90);
    timerStatusPeriodic(100);
    timerPollPeriodic(100);
    // Not sure the following is worth the effort.
    // It is probably better to wait and gang up any failures together.
    missingPDU_Indicator(false);
    //missingPDU_Indicator(true);	// Trigger a status report immediately on missing pdu.
}

// 25.331 13.8 for RB1, RB2, RB3
void RBInfo::defaultConfig0CFRb(unsigned rbn)
{
    rb_Identity(rbn);
    if (rbn == 1) {
        ul_RLC_Mode(URlcModeUm);
        dl_RLC_Mode(URlcModeUm);
        // According to the RRC spec, this mode is illegal for UM mode! Gotta love it.
        // See comments at TransmissionRlcDiscard.
        //transmissionRLC_DiscardMode(TransmissionRlcDiscard::NoDiscard);
        transmissionRLC_DiscardMode(TransmissionRlcDiscard::TimerBasedWithoutExplicitSignaling);
        // Our version of ASN does not transmit the UM RLC LI size, so dont do this.
        //dl_UM_RLC_LI_size(7);
    } else {
        defaultConfigSrbRlcAm();
    }

#if USE_RBMAPPINGINFO	// Unneeded now.
    ul_TransportChannelType(TrChRACHType);
    ul_logicalChannelIdentity(rbn);

    // rb-MappingInfo
    // Two options, depending on if uplink is DCH or RACH.
    rlc_SizeList(eExplicitList);
    rlc_SizeIndex(1);
    mac_LogicalChannelPriority(rbn);
    dl_TransportChannelType(TrChFACHType);
    dl_logicalChannelIdentity(rbn);
#endif
};

// 25.331 13.7 Default configuration number 3, for voice.
// SRBS: RB1 thru RB3 and data: RB5, RB6, RB7.
void RBInfo::defaultConfig3Rb(unsigned rbn)
{
    rb_Identity(rbn);
    switch (rbn) {
    case 1:				// SRB1, used to communicate with RRC itself.
        ul_RLC_Mode(URlcModeUm);
        dl_RLC_Mode(URlcModeUm);
        // Transmission RLC Discard Mode is 'NotConfigured'.
        break;
    case 2: case 3:		// SRB2 and SRB3.
        // This is the config from the manual, but I think we could just
        // use defaultConfigSrbRlcAm();
        ul_RLC_Mode(URlcModeAm);
        transmissionRLC_DiscardMode(TransmissionRlcDiscard::NoDiscard);
        transmissionWindowSize(128);
        maxDat(15);
        timerRST(300);
        max_RST(1);
        // >>pollingInfo
        lastTransmissionPDU_Poll(false);	// redundant, this is the default.
        lastRetransmissionPDU_Poll(false);	// redundant, this is the default.
        timerPollPeriodic(300);
        dl_RLC_Mode(URlcModeAm);
        receivingWindowSize(128);
        //dl_RLC_PDU_size(128);	not specified?
        inSequenceDelivery(true);
        // >>dl_RLC_StatusInfo
        timerStatusProhibit(100);
        missingPDU_Indicator(false);
        timerStatusPeriodic(300);
        //rlc_OneSidedReEst(false);	// Not in the old spec?
        break;
    case 4: assert(0);
    case 5: case 6: case 7:	// These are the AMR voice RBs.
        ul_RLC_Mode(URlcModeTm);
        ul_segmentationIndication(false);
        dl_RLC_Mode(URlcModeTm);
        dl_segmentationIndication(false);
        break;
    default: assert(0);
    }

    // As of 9-2012, this is the only setup that uses non-multiplexed channels
    // so it is the only one one that uses RBMappingInfo.
    // This is the new RBMappingInfo.
    // It must match what is specified in the defineTrCh for these TrCh in
    // TrChConfig setup, see defineTrCh().
    // Originally I started to implement UMTS RBMappingInfo, below, but then I simplified:
    // Took all the separate RBMappingInfo out and put it in these variables:
    // o mTcIsMultiplexed and mTcRbId in the TrChInfo.
    // o mTrChAssigned in the RBInfo.
    // All the other RBMappingInfo defaults, eg, logical channel mapping to rbid is 1-to-1
    // and the rlc size info is we always use all sizes.
    switch (rbn) {
    case 1: case 2: case 3:		// SRB1,2,3 multiplexed on TrCh 4.
        setTransportChannelIdentity(4); break;
    case 5:		// AMR voice channels on RB5,6,7 mapped to trch1,2,3
        setTransportChannelIdentity(1); break;
    case 6:
        setTransportChannelIdentity(2); break;
    case 7:
        setTransportChannelIdentity(3); break;
    default: assert(0);
    }

#if USE_RBMAPPINGINFO	// Unneeded now.
    // rb-MappingInfo
    // pats note: The ul and dl are identical, and probably always will be.
    UL_LogicalChannelMappings(1);
    ul_TransportChannelType(TrChUlDCHType);
    dl_TransportChannelType(TrChDlDCHType);
    switch (rbn) {
    case 1: case 2: case 3: case 4:	// All SRBs on TrCh 4, which is multiplexed.
        ul_transportChannelIdentity(4);
        dl_transportChannelIdentity(4);
        break;
    case 5:		// AMR codec class "A" bits on TrCh 1.
        ul_transportChannelIdentity(1);
        dl_transportChannelIdentity(1);
        break;
    case 6:		// AMR codec class "B" bits on TrCh 2.
        ul_transportChannelIdentity(2);
        dl_transportChannelIdentity(2);
        break;
    case 7:		// AMR codec class "C" bits on TrCh 3.
        ul_transportChannelIdentity(3);
        dl_transportChannelIdentity(3);
        break;
    default:	break;	// N/A
    }
    ul_logicalChannelIdentity(rbn);	// Only needed for RB1-3
    dl_logicalChannelIdentity(rbn);
    // >DL-logicalChannelMappingList
    MappingOption(1);
    rlc_SizeList(eConfigured);
    mac_LogicalChannelPriority(rbn <= 3 ? rbn : 5);
#endif
};



