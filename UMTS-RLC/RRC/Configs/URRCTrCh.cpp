#include "URRCTrCh.h"
#include "UMTSPhCh.h"

static const int cSamsungTest = 0;
const unsigned gFrameLen = 38400;


unsigned TTICode2NumFrames(TTICodes ttiCode)	// return 1,2,4,8
{
    switch (ttiCode) {
    case TTI10ms: return 1;
    case TTI20ms: return 2;
    case TTI40ms: return 4;
    case TTI80ms: return 8;
    default: assert(0);
    }
}
unsigned TTICode2TTI(TTICodes ttiCode)			// return 10,20,40,80
{
    return TTICode2NumFrames(ttiCode)*10;
}

TTICodes TTI2TTICode(unsigned tti)
{
    switch (tti) {
    case 10: return TTI10ms;
    case 20: return TTI20ms;
    case 40: return TTI40ms;
    case 80: return TTI80ms;
    default:
        LOG(ERR) <<"Invalid TTI:"<<tti;
        assert(0);	// kinda harsh, but it should be fixed before we proceed.
    }
}

static int quantizeDchTBSize(int tbsize);
int quantizeRlcSize(bool common, int tbsize);

// This is where we make the descision about the number of tranport blocks and
// parity bits in each RACH transport format, so this is where we determine
// all the programming for the FEC stack.
// (pat) Harvind put in the TBSize argument to over-ride the default here for testing,
// but it is not used anywhere.
void TrChConfig::configRachTrCh(int ulSF,TTICodes ulTTICode,int ulPB, int TBSize)
{
    // The "RLC-Size" specified in the TFS is a magical term defined in 25.331 8.5.1;
    // for FACH it is the Transport Block size.

    // For now, just one TB.
    // Work from the radio-frame size up to determine the ideal TB [Transport Block] size.
    unsigned numRadioFrames = TTICode2NumFrames(ulTTICode);
    unsigned ultti = TTICode2TTI(ulTTICode);	// 10,20,40,80
    unsigned radioFrameSize = gFrameLen / ulSF;
    unsigned ulTotalSize = radioFrameSize * numRadioFrames;
    // Use only convolutional 1/2 rate coding.
    // This takes code block segmentation into account,
    // TODO: code block segmentation could add some filler bits, which is not supported here.
#if 0
    unsigned requestedTBSize = TBSize;
    if (!requestedTBSize) {
        requestedTBSize = R2DecodedSize(ulTotalSize) - ulPB;
        if (cSamsungTest) {
            // Maybe it wants us to subtract out the mac header for use on DCCH.
            // We will have to rate-match either CCCH or DCCH or both.
            requestedTBSize -= 24;
        }
    }
    unsigned ulTBSize = quantizeRlcSize(true,requestedTBSize);
#endif
    unsigned ulTBSize;
        unsigned maxTBSize = R2DecodedSize(ulTotalSize) - ulPB;
    if (!TBSize || (int)maxTBSize < TBSize)
        ulTBSize = quantizeRlcSize(true,maxTBSize);
    else
        ulTBSize = quantizeRlcSize(true,TBSize);
        //int ulMaxTBs = (maxTBSize + ulPB) / (ulTBSize + ulPB); // each TB has an associated parity block, so need to account fo that

    std::string inform(format("RACH radioFrameSize=%d total=%d tbsize=%d",radioFrameSize,ulTotalSize,ulTBSize));
    LOG(INFO) << inform;
    std::cout << inform << std::endl;

    // Later we will work back down to get the actual coded block size, which will have
    // to be rate-matched if it no longer equals the ulTotalSize.

    // Okey dokey, lets do it:
    // Note: The rate-matching in the default configs is 200,
    // Update: After reading 25.212 4.2.7: Rate Matching, I believe
    // the RM attribute has no absolute meaning; its meaning is only
    // relative to the RM attribute of other TrCh that will be transmitted
    // in the same CCTrCh in the same TTI.
    int rm = 256;
    // We only use trch 1; always multiplexed
        RrcTfs *ultfs = ul()->defineTrCh(TrChRACHType,1,true)->getTfs();
        ultfs->setCommonCh()
            ->setSemiStatic(ultti,Convolutional,CodingRate::CodingRate_half,rm,ulPB)
            // Wrong; The RACH never sends an empty frame - if they
            // have nothing to send, they are silent.
            //->addTF(ulTBSize,0)	// Empty frame
            // Danger Will Robinson:  For RACH only, the default TF (this one)
            // is referenced in the RBMappingInfo, so if you change the position
            // of this TF, you must change that also.
            ->addTF(ulTBSize,1);	// One frame.
    ul()->setCTFCSize(2)
        ->addTFC(0)->checkCTFC(0);
        //->addTFC(1)->checkCTFC(1);

        for (unsigned n = 2; n <= 2 /*ulMaxTBs*/; n++) {
                ultfs->addTF(ulTBSize,n);       // One frame.
                ul()->addTFC(n-1)->checkCTFC(n-1);
        }


}

void TrChConfig::configFachTrCh(int dlSF,TTICodes dlTTICode,int dlPB, int TBSize)
{
    // The "RLC-Size" specified in the TFS is a magical term defined in 25.331 8.5.1;
    // for FACH it is the Transport Block size.

    // For now, just one TB.
    // Work from the radio-frame size up to determine the ideal TB [Transport Block] size.
    unsigned numRadioFrames = TTICode2NumFrames(dlTTICode);
    unsigned dltti = TTICode2TTI(dlTTICode);	// 10,20,40,80
    unsigned radioFrameSize = getDlRadioFrameSize(SCCPCHType,dlSF);
    unsigned dlTotalSize = radioFrameSize * numRadioFrames;
    // Use only convolutional 1/2 rate coding.
#if 0
    //unsigned requestedTBSize = R2DecodedSize(dlTotalSize) - dlPB;
    unsigned requestedTBSize = TBSize;
    if (!requestedTBSize) {
        requestedTBSize = R2DecodedSize(dlTotalSize) - dlPB;
        if (cSamsungTest) {
            // Maybe it wants us to subtract out the mac header for use on DCCH.
            // We will have to rate-match either CCCH or DCCH or both.
            requestedTBSize -= 24;
        }
    }
    unsigned dlTBSize = quantizeRlcSize(true,requestedTBSize);
#endif
        unsigned dlTBSize;
        unsigned maxTBSize = R2DecodedSize(dlTotalSize) - dlPB;
        if (!TBSize || (int)maxTBSize < TBSize)
                dlTBSize = quantizeRlcSize(true,maxTBSize);
        else
                dlTBSize = quantizeRlcSize(true,TBSize);

    int rm = 256;

    std::string inform(format("FACH SF=%d radioFrameSize=%d total=%d tbsize=%d\n",dlSF,radioFrameSize,dlTotalSize,dlTBSize));
    LOG(INFO) << inform;
    std::cout << inform << std::endl;

    // We only use trch 1; always multiplexed
    dl()->defineTrCh(TrChFACHType,1,true)
        ->getTfs()
            ->setCommonCh()
            ->setSemiStatic(dltti,Convolutional,CodingRate::CodingRate_half,rm,dlPB)
            ->addTF(dlTBSize,0)
            ->addTF(dlTBSize,1);	// One frame.
    dl()->setCTFCSize(2)
        ->addTFC(0)->checkCTFC(0)
        ->addTFC(1)->checkCTFC(1)->setPower(PowerOffset(11));
}
DlTrChInfo *DlTrChInfo::bler_QualityValue(double val) { mDCHQualityTarget = val; return this; }

// Create the simplest TFS and TFCS we can for a DCH for PS [internet] services.
// The only thing the dch knows a-priori is the spreading factor.
// If the requested ulTBSize or dlTBSize is 0, make a trivial TFCS with just one block of
// the maximum size that will fit in the TTI and SF, otherise (it is non-zero) make a TFCS with as many
// TB of that size as will fit in the TTI and SF.
// Return true if we could use tbsize or false if we had to reduce tbsize to fit in the SF.
// NO! pat 1-24-2013: Make sure to use different TrCh for DCH than we used for RACH/FACH.  This is needed because
// sendRadioBearerRelease() now assumes different TrCh are used in CELL_FACH and CELL_DCH states.

/*
bool TrChConfig::configDchPS(DCHFEC *dch, TTICodes tticode, unsigned pb, bool useTurbo, unsigned ulTBSize, unsigned dlTBSize)
{
    TTICodes dlTTICode = tticode;
    TTICodes ulTTICode = tticode;
    unsigned dlPB = pb;
    unsigned ulPB = pb;
    int rm = 256;
    bool result = true;		// assume success.

    // The g++ version 4.6.3 compiler has a bug in -O0 mode only that it cannot cope with this constant expression,
    // so we have to fix it by adding a bunch of extraneous casts to convince it that yes, this really is
    // a constant integral expression:
    CodingType codingtype = (CodingType)(useTurbo ? (int)Turbo : (int)Convolutional);

    {
    unsigned ultti = TTICode2TTI(ulTTICode);	// 10,20,40,80
    unsigned ulNumRadioFrames = TTICode2NumFrames(ulTTICode);
    unsigned ulRadioFrameSize = dch->getUlRadioFrameSize();
    unsigned ulTotalSize = ulRadioFrameSize * ulNumRadioFrames;
    // R2DecodedSize takes the code block segmentation filler bits into account.
    unsigned ulMaxTBs;
    unsigned ulDecodedSize = (useTurbo ? TurboDecodedSize(ulTotalSize) : R2DecodedSize(ulTotalSize)) - ulPB; // assumes single trans. block
    if (ulTBSize) {
        // Check assumption that TB size is small enough so no code block segmentation is necessary.
        assert(ulTBSize <= (useTurbo ? 5114 : 504));
                ulMaxTBs = (ulDecodedSize + ulPB) / (ulTBSize + ulPB); // each TB has an associated parity block, so need to account fo that
        if (ulMaxTBs == 0) {
            // Back off to a tbsize that works.
            unsigned ulMaxTBSize = quantizeDchTBSize(ulDecodedSize);
            LOG(ERR) << "Internal DCH config: uplink TB size="<<ulTBSize<<" is too large for SF="<<dch->getPhCh()->getUlSF()
                    << " using TBsize="<<ulMaxTBSize;
            result = false;
            ulTBSize = ulMaxTBSize;
        } else if (ulMaxTBs >= maxTbPerTrCh) {
            LOG(ERR) << "Internal DCH config: with uplink TB size="<<ulTBSize<<" then numTBs="<<ulMaxTBs<<" which exceeds maximum="<<maxTbPerTrCh<<" using max instead";
            ulMaxTBs = maxTbPerTrCh - 1;
        }
    } else {
        // Round the TB size down to the next permitted value.
        ulTBSize = quantizeDchTBSize(ulDecodedSize);
        ulMaxTBs = 1;
    }
// 12-18: pat.  There is a bug in SF=4 that prevents it from working reliably with the full size TFS.
// If you set this flag it limits the TFS size and works at SF=4 at a reduced bandwidth.
#define TEST_SF4 0
#if TEST_SF4
    ulMaxTBs = 4;	// Reduce to match SF=8.
#endif
    unsigned ulTotalTB = ulMaxTBs*ulTBSize;
    std::string informul(format("configDchPS: UL SF=%u RF=%ux%u=%u pb=%u turbo=%d maxprecoded=%u tb=%ux%u=%u coded=%u\n",
        dch->getPhCh()->getUlSF(),ulNumRadioFrames,ulRadioFrameSize,ulTotalSize,
        ulPB,useTurbo,ulDecodedSize,ulMaxTBs,ulTBSize,ulTotalTB,
        useTurbo? TurboEncodedSize(ulTotalTB) : R2EncodedSize(ulTotalTB)));
    LOG(INFO) << informul;
    std::cout << informul << std::endl;

    // Okey dokey, lets do it:
    // Add the TF and TFC for 0 and 1 TBs.
    // NO {pat 1-24-2013: Change the DCH TrCh from 1 to 2, so TrCH == 1 used for RACH/FACH and TrCh == 2 used for DCH.
    // The UE remembers the TrCh programming until it goes to idle mode, so using different TrCh makes
    // the radio bearer release easier; we dont have to reprogram the TrCh, we just switch it using the RBMappingInfo. }
    RrcTfs *ultfs = ul()->defineTrCh(TrChUlDCHType,1,true)->getTfs();
        ultfs->setDedicatedCh()
            ->setSemiStatic(ultti,codingtype,ASN::CodingRate_half,rm,ulPB)
            ->addTF(ulTBSize,0)		// Empty frame.
            ->addTF(ulTBSize,1);	// One frame.

    // Must +1 because we start numbering from 0.
    ul()->setCTFCSize(floor(log2(ulMaxTBs+1))+1);
    ul()->addTFC(0)->checkCTFC(0)
        ->addTFC(1)->checkCTFC(1);

    // If more than 1 TB, add those TF and TFC.
    for (unsigned n = 2; n <= ulMaxTBs; n++) {
        ultfs->addTF(ulTBSize,n);	// N frames.
        ul()->addTFC(n);
    }
    } // local context

    {
    unsigned dltti = TTICode2TTI(dlTTICode);	// 10,20,40,80
    unsigned dlNumRadioFrames = TTICode2NumFrames(dlTTICode);
    unsigned dlRadioFrameSize = dch->getDlRadioFrameSize();
    unsigned dlTotalSize = dlRadioFrameSize * dlNumRadioFrames;
    unsigned dlDecodedSize = (useTurbo ? TurboDecodedSize(dlTotalSize) : R2DecodedSize(dlTotalSize)) - dlPB;
    unsigned dlMaxTBs;
    if (dlTBSize) {
        // Check assumption that TB size is small enough so no code block segmentation is necessary.
        assert(dlTBSize <= (useTurbo ? 5114 : 504));
        dlMaxTBs = (dlDecodedSize + dlPB) / (dlTBSize + dlPB); // each TB has an associated parity block, so need to account fo that
        if (dlMaxTBs == 0) {
            unsigned dlMaxTBSize = quantizeDchTBSize(dlDecodedSize);
            LOG(ERR) << "Internal DCH config: downlink TB size="<<dlTBSize<<" is too large for SF="<<dch->getPhCh()->getDlSF()
                    << " using TBsize="<<dlMaxTBSize;
            result = false;
            dlTBSize = dlMaxTBSize;
            dlMaxTBs = 1;
        } else if (dlMaxTBs >= maxTbPerTrCh) {
            LOG(ERR) << "Internal DCH config: with uplink TB size="<<dlTBSize<<" then numTBs="<<dlMaxTBs<<" which exceeds maximum="<<maxTbPerTrCh<<" using max instead";
            dlMaxTBs = maxTbPerTrCh - 1;
        }
    } else {
        // Round the TB size down to the next permitted value.
        dlTBSize = quantizeDchTBSize(dlDecodedSize);
        dlMaxTBs = 1;
    }

    unsigned dlTotalTB = dlMaxTBs*dlTBSize;
#if TEST_SF4
    if (dlMaxTBs > 12) dlMaxTBs = 12;	// SF=4 doesnt work, so reduce.
#endif

    std::string inform(format("configDchPS: DL SF=%u RF=%ux%u=%u pb=%u turbo=%d maxprecoded=%u tb=%ux%u=%u coded=%u\n",
        dch->getPhCh()->getDlSF(),dlNumRadioFrames,dlRadioFrameSize,dlTotalSize,
        dlPB,useTurbo,dlDecodedSize,dlMaxTBs,dlTBSize,dlTotalTB,
        useTurbo? TurboEncodedSize(dlTotalTB) : R2EncodedSize(dlTotalTB)));
    LOG(INFO) << inform;
    std::cout << inform << std::endl;

    // pat 1-24-2013: See comments for ul()->defineTrCH above.
    RrcTfs *dltfs = dl()->defineTrCh(TrChDlDCHType,1,true)->getTfs();
        dltfs->setDedicatedCh()
            ->setSemiStatic(dltti,codingtype,ASN::CodingRate_half,rm,dlPB)
            ->addTF(dlTBSize,0)	// Empty frame.
            ->addTF(dlTBSize,1);	// One frame.
    // Must +1 because we start numbering from 0.
    dl()->setCTFCSize(floor(log2(dlMaxTBs+1))+1)
        ->addTFC(0)->checkCTFC(0)
        ->addTFC(1)->checkCTFC(1);

    // If more than 1 TB, add those TF and TFC.
    for (unsigned n = 2; n <= dlMaxTBs; n++) {
        dltfs->addTF(dlTBSize,n);	// One frame.
        dl()->addTFC(n);
        //tcdump(); // This was ok:
    }
    } // local context
    return result;
}

*/

// From 25.331 13.7: Parameter values for default radio configurations.
// This setup is for voice, 4 TrCH: 3 for AMR and 1 for signalling..
// The "3" is the setup number from the documentation, not the number of TrCh.
// The DCH must be SF=128 or higher.
void TrChConfig::defaultConfig3TrCh()
{
    // The TrCh format in the default configs is confusing.
    // The first line labeled tf0, tf1, etc, has everything you need,
    // but the size and number of blocks are redundantly repeated
    // in sizeType and numberOfTbSizeList.
    // NOTE: These have Rate Matching attributes that we do not support.

    // TrCH Information per TrCH
    // TrCHInformationPerTrCH();

    // Define 4 TrCh.
    // TrCh 1,2,3 not multiplexed - carries only this logical channel.
    ul()->defineTrCh(TrChUlDCHType,1,false,5) // AMR codec class "A" bits on RB5 on TrCh 1.
        ->getTfs()
            ->setDedicatedCh()
            ->setSemiStatic(20,Convolutional,CodingRate::CodingRate_third,200,12)
            ->addTF(81,0)->addTF(39,1)->addTF(81,1);
    ul()->defineTrCh(TrChUlDCHType,2,false,6)	 // AMR codec class "B" bits on RB6 on TrCh 2.
        ->getTfs()
            ->setDedicatedCh()
            ->setSemiStatic(20,Convolutional,CodingRate::CodingRate_third,190,0)
            ->addTF(103,0)->addTF(103,1);
    ul()->defineTrCh(TrChUlDCHType,3, false,7) // AMR codec class "B" bits on RB7 on TrCh 3.
        ->getTfs()
            ->setDedicatedCh()
            ->setSemiStatic(20,Convolutional,CodingRate::CodingRate_half,235,0)
            ->addTF(60,0)->addTF(60,1);
    // TrCh 4 will carry multiplexed SRB1,2,3.
    ul()->defineTrCh(TrChUlDCHType,4,true)
        ->getTfs()
            ->setDedicatedCh()
            ->setSemiStatic(40,Convolutional,CodingRate::CodingRate_half,160,16)
            ->addTF(144,0)->addTF(144,1);

    // Common TrCh info
    ul()->setCTFCSize(6)		// TODO: Does this really need to be 6?
        ->addTFC(0,0,0,0)->checkCTFC(0)
        ->addTFC(1,0,0,0)->checkCTFC(1)
        ->addTFC(2,1,1,0)->checkCTFC(11)
        ->addTFC(0,0,0,1)->checkCTFC(12)
        ->addTFC(1,0,0,1)->checkCTFC(13)
        ->addTFC(2,1,1,1)->checkCTFC(23)->setPower(PowerOffset(11));


    // The spec says only tf0 is different and specified as 1x0 instead of 81x0.
    // So what is up with that?  infinite*0 is still 0.
    // Maybe has to do with tfci-existence = true in uplink but = false in downlink.
    // In that case, why didnt they just use 1x0 in ul above?
    // Anyway, here it is:
    dl()->defineTrCh(TrChDlDCHType,1,false,5)
        ->bler_QualityValue(7e-3)
        ->getTfs()
            ->setDedicatedCh()
            ->setSemiStatic(20,Convolutional,CodingRate::CodingRate_third,200,12)
            ->addTF(1,0)->addTF(39,1)->addTF(81,1);

    dl()->defineTrCh(TrChDlDCHType,2,false,6)->copyUplink(ul(),2) ->bler_QualityValue(7e-3);
    dl()->defineTrCh(TrChDlDCHType,3,false,7)->copyUplink(ul(),3) ->bler_QualityValue(7e-3);
    dl()->defineTrCh(TrChDlDCHType,4,true)->copyUplink(ul(),4) ->bler_QualityValue(7e-3);

    // dl CTFCs are unspecified, but must be the same.  Do we need to specify them?  Seems like we must.
    dl()->setCTFCSize(6)		// TODO: Is this really 6?
        ->addTFC(0,0,0,0)->checkCTFC(0)
        ->addTFC(1,0,0,0)->checkCTFC(1)
        ->addTFC(2,1,1,0)->checkCTFC(11)
        ->addTFC(0,0,0,1)->checkCTFC(12)
        ->addTFC(1,0,0,1)->checkCTFC(13)
        ->addTFC(2,1,1,1)->checkCTFC(23)->setPower(PowerOffset(11));	// What does setpower do here?


    /***
    // TODO: What is this?  It is used instead of a downlink transport channel id.
    ULTrCH_Id(1,1);
    ULTrCH_Id(2,2);
    ULTrCH_Id(3,3);
    ULTrCH_Id(4,4);
    ****/

    /***
    // PhyCH INFORMATION FDD
    //ul-DPCH-InfoPredef
    //>ul-DPCH-PowerControlInfo
    //powerControlAlgorithm(1);
    //tpcStepSize(1);	// 1dB
    // Default config needs exactly this puncturing to fit in SF=128, but we will use lower rate coding to avoid the need for puncturing.
    // puncturingLimit(0.88);
    //DL-CommonInformationPredef
    //>dl-DPCH-InfoCommon
    spreadingFactor(128);
    tfciExistence(false);
    pilotBits(4);
    posititionFixed(Fixed);
    ***/

    // Skip a bunch of TDD information.
};

// The RRC spec has an option to set mChoiceDlParameters to tell the phone to copy the downlink params,
// but for out purposes we need to copy the TFS so we can program our own stack anyway,
// and since the RRC option is just to save room in the message we are not going to implement it,
// we are going to genuinely copy the uplink programming.
// Since we dont have an uplink or parent pointer here, the easiest way was to pass the ul being copied.
DlTrChInfo *DlTrChInfo::copyUplink(UlTrChList *ul, unsigned trch1based) {
    //mChoiceDlParameters = eSameAsUL; // the RRC sameAsUL option is not implemented
    mTfs = *ul->getTfs(trch1based-1);
    return this;
}

unsigned RrcTfc::getNumTrCh() const { return mTrChList->getNumTrCh(); }

RrcTf *RrcTfc::getTf(unsigned trchid) { return &mTfList[trchid]; }

TrChInfo *RrcTfc::getTrChInfo(TrChId tcid) { return mTrChList->getTrChInfo(tcid); }

// Set this TFC.  Up to four TFIndex can be specified for up to four TrCh.
void RrcTfc::setTfc(TrChList *chlist, TfIndex tfa, TfIndex tfb, TfIndex tfc, TfIndex tfd)
{
    mTrChList = chlist;	// Save for future reference.
    // These may not all be used, the unused ones just default to 0.
    unsigned nTrCh = chlist->getNumTrCh();
    // Error check:
    assert(tfa < chlist->getTfs(0)->getNumTf());
    if (nTrCh >= 2) assert(tfb < chlist->getTfs(1)->getNumTf());
    if (nTrCh >= 3) assert(tfc < chlist->getTfs(2)->getNumTf());
    if (nTrCh >= 4) assert(tfd < chlist->getTfs(3)->getNumTf());
    mTfList[0] = RrcTf(chlist->getTfs(0),tfa);
    mTfList[1] = RrcTf(nTrCh >= 2 ? chlist->getTfs(1) : 0,tfb);
    mTfList[2] = RrcTf(nTrCh >= 3 ? chlist->getTfs(2) : 0,tfc);
    mTfList[3] = RrcTf(nTrCh >= 4 ? chlist->getTfs(3) : 0,tfd);
    // Calculate ctfc 25.331 14.10
    unsigned p = 1;
    unsigned ctfc = 0;
    unsigned tfi[4]; tfi[0] = tfa; tfi[1] = tfb; tfi[2] = tfc; tfi[3] = tfd;
    for (unsigned ch = 0; ch < nTrCh; ch++) {
        ctfc += p * tfi[ch];
        p = p * chlist->getTfs(ch)->getNumTF();
    }
    mCTFC = ctfc;
    mPowerOffset.mPresent = false;	// redundant - default initializer does this.
}

// This returns a size of the TFC that is related neither to the TB size nor the Rlc payload size,
// but suitable only for comparing to another ConfusingSize.
unsigned RrcTfc::getTfcSize()
{
    unsigned numTrCh = mTrChList->getNumTrCh();
    // Get the sum of the rlc sizes of all the transport blocks in this TFC.
    unsigned total = 0;
    for (TrChId i = 0; i < numTrCh; i++) {
        total += mTfList[i].getTfTotalSize();
    }
    return total;
}


RrcTfcs *RrcTfcs::addTFC2(TrChList *chlist, TfIndex tfa, TfIndex tfb, TfIndex tfc, TfIndex tfd)
{
    assert(mNumTfc < maxTfc);
    mTfcList[mNumTfc].setTfc(chlist, tfa, tfb, tfc, tfd);
    mNumTfc++;
    return this;
}

static int quantizeCTFCSize (int ctfcSize)
{
//quantize up to 2, 4, 6, 8, 12, 16
    if (ctfcSize > 12) return 16;
    if (ctfcSize > 8) return 12;
    if (ctfcSize > 6) return 8;
    if (ctfcSize > 4) return 6;
    if (ctfcSize > 2) return 4;
    return 2;
}



// Return the transport block size rounded down to the size accepted by
// the rlc-size IE parameter.  It is different for common and dedicated channels.
// Described in 25.331 10.3.5.23 Transport Format Set.
int quantizeRlcSize(bool common, int tbsize)
{
    if (!common) {
        // 16..5000 by step of 8
        // FIXME: Need to do this?  Everybody seems to say that PDU size is fixed to 336 for data.
        return (tbsize / 8)*8;
    } else {
        // More abstractly, this function should go through
        // 25.212 FDD Multiplexing and coding and take all of the extra padding
        // into account, including code block segmentation, radio frame segmentation.

        // 48..296 step 8,
        // 312..1320 step 16;  note: 312 is not evenly divisible by 16.
        // 1384..4968 step 64; note: 1384 is not evenly divisible by 64.
        printf("TBSIZE: %d\n",tbsize);
        if (tbsize <= 48) {
            return 48;
        } else if (tbsize <= 296) {
            return (tbsize/8)*8;
        } else if (tbsize < 312) {
            return 296;
        } else if (tbsize <= 1320) {
            return ((tbsize-312)/16)*16 + 312;
        } else if (tbsize < 1384) {
            return 1320;
        } else if (tbsize <= 4968) {
            return ((tbsize-1384)/64)*64 + 1384;
        } else {
            return 4968;
        }
    }
}

// We need to quantize the "RLC-Size" which is a magical term defined in 25.331 8.5.1;
// Presumably that is so you can use the same RLC blocks on different physical channels.
// For dedicated channels, instead of quantizing the TB-size,
// we quantize the actual rlc-size which is the TB-size less the mac header size,
// which for multiplexed channels is 4 bits.
// All of our channels are multiplexed.
static int quantizeDchTBSize(int requestedTBSize)
{
    unsigned requestedRlcSize = requestedTBSize - 4;
    unsigned finalTBSize = quantizeRlcSize(false,requestedRlcSize) + 4;
    return finalTBSize;
}

bool RrcTfs::isMacMultiplexed() { return mTrChPtr->mTcIsMultiplexed; }
RbId RrcTfs::getNonMultiplexedRbId() { return mTrChPtr->mTcRbId; }

//unsigned RrcTfs::getCodedBlockSize(int tfnum)
//{
//	unsigned tbsize = getTBSize(tfnum);
//	unsigned tbnum = getNumTB(tfnum);
//	// Each transport block gets parity attched, then concatenated together.
//	// This is fed down and may need to be rate-matched or DTX inserted to fit
//	// in the radio frame(s).
//	// TODO: code block segmentation, which may add some filler bits.
//	unsigned totalsize = tbnum * (tbsize + getPB());
//	switch (mSemiStaticTFInfo.mTypeOfChannelCoding) {
//	case Convolutional:
//		// Dont even check the rate - we only support 1/2.
//		return R2EncodedSize(totalsize);
//	case Turbo:
//		return TurboEncodedSize(totalsize);
//	default: assert(0);
//	}
//}
