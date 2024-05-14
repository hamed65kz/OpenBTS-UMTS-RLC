#include "RrcMasterChConfig.h"


RBInfo *RrcMasterChConfig::setSRB(unsigned rbid)
{
    assert(rbid<=4);
    if (rbid >= mNumRB) { mNumRB = rbid+1; }
    mRB[rbid].mRbId = rbid;	// Kinda redundant, but also marks this RB as valid.
    return &mRB[rbid];
}

RBInfo *RrcMasterChConfig::setRB(unsigned rbid,CNDomainId domain)
{
    assert(rbid > 4);
    assert(rbid<gsMaxRB);
    if (rbid >= mNumRB) { mNumRB = rbid+1; }
    mRB[rbid].mRbId = rbid;	// Kinda redundant, but also marks this RB as valid.
    mRB[rbid].mPsCsDomain = domain;
    return &mRB[rbid];
}

// For PS services the UE may request several IP connections at different times,
// which might generate several RAB messages.
// Not sure if we should/can send the SRBs on subsequent messages.
// For now, just throw the SRBs in every message and hope the UE ignores them if it needs to.
// Note that the config is only for the new RAB being allocated -
// pre-existing RABs with different ids inside the UE are not supposed to be affected.

//void RrcMasterChConfig::rrcConfigDchPS(DCHFEC *dch, int RABid, bool useTurbo)
//{
//    // Define a simple multiplexed TrCh of width for dch.
//    if (this->mTrCh.dl()->getNumTrCh() == 0) {
//        this->mTrCh.configDchPS(dch, TTI10ms, 16, useTurbo, 340+40, 340);
//    } else {
//        // TrCh setup already configured.
//        // We may be defining a second RAB for a second PDPContext.
//        // FIXME: If the QOS or dch has changed or we are adding PS to CS services,
//        // we need to add the TrCh to the existing configuration.
//    }
//    //this->mTrCh.tcdump();
//
//    // The RLC for the SRB will take the rlc size from the TrCh,
//    // and fill it up with filler after the miniscule message.
//    this->setSRB(1)->defaultConfig0CFRb(1);
//    this->setSRB(2)->defaultConfig0CFRb(2);
//    this->setSRB(3)->defaultConfig0CFRb(3);
//
//    // Now what about the RAB?
//    assert(RABid >= 5 && RABid <= 15);
//    //this->addRAB(rbid,CNDomainId)
//    // TODO: We may want to use RLC-UM for a PFT for TCP/UDP.  Clear?
//    this->setRB(RABid,PSDomain)->defaultConfigRlcAmPs();
//    //this->mTrCh.tcdump();
//}

// The DCH must be SF=128 or higher.
void RrcMasterChConfig::rrcConfigDchCS()
{
    this->mTrCh.defaultConfig3TrCh();
    this->setSRB(1)->defaultConfig3Rb(1);
    this->setSRB(2)->defaultConfig3Rb(2);
    this->setSRB(3)->defaultConfig3Rb(3);
    // AMR voice on RB5,6,7
    this->setRB(5,CSDomain)->defaultConfig3Rb(5);
    this->setRB(6,CSDomain)->defaultConfig3Rb(6);
    this->setRB(7,CSDomain)->defaultConfig3Rb(7);
}
