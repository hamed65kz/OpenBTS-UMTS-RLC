#include "URlcPair.h"
#include "Recv/URlcRecvAm.h"
#include "Recv/URlcRecvUm.h"
#include "Recv/URlcRecvTm.h"

#include "Tran/URlcTransAm.h"
#include "Tran/URlcTransUm.h"
#include "Tran/URlcTransTm.h"

#include "URlcAm.h"

#include "../Ue/UEInfo.h"

const char*URlcMode2Name(URlcMode mode)
{
    switch (mode) {
    case URlcModeTm: return "TM";
    case URlcModeUm: return "UM";
    case URlcModeAm: return "AM";
    default: assert(0);
    }
}


// How many bits are in the MAC header?
unsigned macHeaderSize(TrChType tctype, ChannelTypeL3 lch, bool multiplexing)
{
    // From 25.321 9.2.1.4 [MAC spec]
    // Table 9.2.1.4: Coding of the tctf [Target Channel Type Field] on RACH for FDD
        // CCCH => 2 bits
        // DCCH or DTCH => 2 bits
    // Table 9.2.1.2: Coding of tctf [Target Channel Type Field] on FACH for FDD:
        // BCCH => 2 bits
        // CCCH => 8 bits
        // DCCH or DTCH => 2 bits
    switch (lch) {
    case BCCHType:
        switch (tctype) {
        case TrChFACHType: return 2;
        case TrChBCHType: return 2;
        default: assert(0);
        }
        break;
    case CCCHType:
        // CCCH on RACH/FACH adds tctf only.
        // (Because this is used only for the initial connection setup message
        // in which the UE id encoded is in the message instead of the MAC header.)
        switch (tctype) {
        case TrChRACHType: return 2;
        case TrChFACHType: return 8;
        default: assert(0);
        }
        break;
    case DCCHType:
    case DTCHType:
        switch (tctype) {
        case TrChDlDCHType:
        case TrChUlDCHType:
            // From 25.321 9.2.1.1 case (a) no multiplexing or (b) multiplexing.
            // 'multiplexing' means multiple logical channels on the same TrCh, which will
            // likely always be true for us except for voice channels.
            return multiplexing ? 4 : 0;
        case TrChRACHType:
        case TrChFACHType:
            // From 25.321 9.2.1.1 case (c) DTCH+DCCH mapped to RACH/FACH: tctf+ueidtype+ueid+c/t
            // 2 bits for tctf from Table 9.2.1.2
            // 2 bits for ue id type
            // 16 bits for ue id of type C-RNTI
            // 4 bits for C/T field (which is Radio Bearer id)
            return 2+2+16+4;
        default: assert(0);
        }
    default: assert(0);
    }
}


// For all channels except BCCH:
// Since we pass the macMutiplexed flag explicitly there are only two different
// cases of logical channel to worry about, CCCH or not-CCCH, so if it
// is not-CCCH pass DCCH, which has same mac header size as DTCH.
// If we start sending BCCH on FACH (why would we ever?) then this needs to change.
unsigned macHeaderSize(TrChType tctype, int rbid, bool multiplexing)
{
    return macHeaderSize(tctype, rbid == 0 ? CCCHType : DCCHType, multiplexing);
}
unsigned computeDlRlcSize(RBInfo *rb, RrcTfs *dltfs)
{
    if (rb->getDlRlcMode() == URlcModeTm) {return 0;}
    TrChInfo *dltc = dltfs->getTrCh();
    unsigned dlmacbits = macHeaderSize(dltc->mTransportChType,rb->mRbId,dltc->mTcIsMultiplexed);

    // For UM and AM there should only be one TB size, although there is
    // sometimes a 0 size as well, which is why we use the MaxTBSize.
    // For TM there could be multiple TB sizes, but we dont care because
    // they just pass through for the MAC to worry about.
    // UM and AM RLC are byte-oriented, so ??
    return (dltfs->getMaxTBSize() - dlmacbits)/8;
}

// Allocate the uplink and downlink RLC entities for this rb.
URlcPair::URlcPair(RBInfo *rb, RrcTfs *dltfs, UEInfo *uep, TrChId tcid)
    : mTcid(tcid)
{
    // We need the mac header size for this particular rb to subtract from the TrCh TBSize.
    // We pass that size to the AM/UM mode configs, and they subtract the RLC AM or UM
    // header size out of this pdusize.
    unsigned dlPduSizeBytes = computeDlRlcSize(rb,dltfs);

    {
    TrChInfo *dltc = dltfs->getTrCh();
    unsigned dlmacbits = macHeaderSize(dltc->mTransportChType,rb->mRbId,dltc->mTcIsMultiplexed);
    PATLOG(4,format("URlcPair(rb=%d,ul=%s,dl=%s,pdusizebits=%d+%d,%s)",
            rb->mRbId, URlcMode2Name(rb->getUlRlcMode()), URlcMode2Name(rb->getDlRlcMode()),
            dlmacbits,dlPduSizeBytes, uep->ueid().c_str()));
    }

    // We do not need to pass a pdu size to the uplink RLC entities because they just
    // assemble whatever size pdus come in.
    switch (rb->getUlRlcMode()) {
    case URlcModeAm: {		// If ul is AM, dl is AM too.
        assert(rb->getDlRlcMode() == URlcModeAm);
        URlcAm *amrlc = new URlcAm(rb,dltfs,uep,dlPduSizeBytes);	// Includes UrlcTransAm and UrlcRecvAm
        mDown = (amrlc);
        mUp =  (amrlc);
        return;
        }
    case URlcModeUm:
        mUp = new URlcRecvUm(rb,dltfs,uep);
        break;
    case URlcModeTm:
        // TODO: Add config to TM?
        mUp = new URlcRecvTm(rb,uep);
        break;
    }
    switch (rb->getDlRlcMode()) {
    case URlcModeAm: assert(0);	// handled above.
        break;
    case URlcModeUm:
        mDown = new URlcTransUm(rb,dltfs,uep,dlPduSizeBytes);
        break;
    case URlcModeTm:
        mDown = new URlcTransTm(rb,uep);
        break;

    }
}

URlcPair::~URlcPair()
{
    if (mDown->mRlcMode == URlcModeAm) {
        // It is RLC-AM, and there is only one combined entity.
        assert(mUp->mRlcMode == URlcModeAm);
        URlcAm *am = dynamic_cast<URlcAm*>(mDown);
        assert(am);
        delete am;
    } else {
        delete mUp;
        delete mDown;
    }
}
