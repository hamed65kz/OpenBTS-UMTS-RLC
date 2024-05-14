#ifndef RRCMASTERCHCONFIG_H
#define RRCMASTERCHCONFIG_H
#include "../../Ue/RBInfo.h"

#include "URRCTrCh.h"


// Master Channel Config.
// Its just an RB and TrCh config combined.
// The RBMappingInfo can specify two mappings for each RB:
// one when mapped to DCH and one when mapped to RACH/FACH;
// therefore we could have two TrChConfig here.



class RrcMasterChConfig : public RrcDefs
{
    static const unsigned sMaxSRB = 3;	// We dont use SRB4.
    //static const unsigned sMaxDRB = 1;	// Will need to be 3 for AMR voice channels.
    static const unsigned sMaxTrCh = 2;	// Minimum 2 for DCCH+DTCH; Will need to be 4 for AMR voice.

    // This is indexed by the RB number, which means that position 0 is not used
    // except for SRB0 in the CCCH MasterChConfig.
    // old :Warning: the RBMappingInfo contains indicies into the TFS buried in the TrCh info
    // 		(but we dont use RBMappingInfo any more)
    RBInfo mRB[gsMaxRB];	// Has RLC info, RBMappingInfo

    public:
    TrChConfig mTrCh;	// This has lists of TrCh, their TFS, and the master TFCS.

    // One greater than the index of the maximum mRB defined; for cch it is only 1.
    // This is used just to limit the loops that look through them.
    unsigned mNumRB;

    RrcMasterChConfig():
        mNumRB(0)
        {}

    // The getRB works for RB or SRB.
    RBInfo *getRB(unsigned rbid) { assert(rbid<mNumRB); return &mRB[rbid]; }
    RBInfo *setSRB(unsigned rbid);
    RBInfo *setRB(unsigned rbid,CNDomainId domain);

    // These use 0 based numbering for TrCh:
    TrChInfo *getUlTrChInfo(TrChId tcid) { return mTrCh.ul()->getTrChInfo(tcid); }
    TrChInfo *getDlTrChInfo(TrChId tcid) { return mTrCh.dl()->getTrChInfo(tcid); }
    unsigned getUlNumTrCh() { return mTrCh.ul()->mNumTrCh; }
    unsigned getDlNumTrCh() { return mTrCh.dl()->mNumTrCh; }

    // TFS are per-TrCh.  TFCS are common for all TrCh.
    RrcTfs *getUlTfs(TrChId tcid = 0) { return mTrCh.ul()->getTfs(tcid); }
    //RrcTfcs *getUlTfcs() { return mTrCh.ul()->getTfcs(); }
    RrcTfs *getDlTfs(TrChId tcid = 0) { return mTrCh.dl()->getTfs(tcid); }
    //RrcTfcs *getDlTfcs() { return mTrCh.dl()->getTfcs(); }

    // Create a DCH channel with the a data channel on the specified RABid.
    //void rrcConfigDchPS(DCHFEC *dch, int RabId, bool useTurbo);

    // Config DCH for CS (circuit-switched, ie, voice).
    // FIXME: Finish this.  I set up the RBInfo and TrChInfo, but we still
    // need to do the other stuff in rrcAllocateRabForPdp.
    void rrcConfigDchCS();

    // TODO: uplink function = none, RLC bypassed
};

#endif
