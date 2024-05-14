#ifndef UMTSPHCH_H
#define UMTSPHCH_H
#include "RrcDefs.h"
#include "../../Utilities/Logger.h"

#include<assert.h>

const unsigned gFrameSlots = 15;

// Note: This duplicates slot information that Harvind has added elsewhere.
// 3GPP 25.211
// table 6: [page 17] Random-access [aka PRACH] message data fields.
//		PRACH is different because the control and data parts are transmitted
//		on two carriers simultaneously; control always uses slot0.
// table 18: [page 36] Secondary CCPCH fields.
// table 1+2 [page 11] uplink DBDCH, DPCCH fields.
// table 11: [page 23] downlink DPDCH and DPCCH fields.
struct SlotFormat {
    int mSlotFormat;	// The index in this table; redundant - must match location in table.
    // int mBitRate;	// We dont need to keep this.
    int mSF;			// We dont need to keep this, but it is a handy comment.
    // int mBitsPerFrame;	// This is always bitsperslot*15.
    int mBitsPerSlot;
    int mNData1;				// may be data1+data2.
    int mNData2;				// if no data2 in table, then just 0.
    int mNPilot;
    int mPilotIndex;		// Index into sDlPilotBitPatternTable;
    int mNTfci;		// Number of raw tfci bits in the slot.
    int mNTpc;		// transmit power control, not used for CCPCH, always 0.

    // Currently the SlotForamt is used only by layer2, so I threw away the data1/data2 info and fbi,
    // but I put those in the tables so it is trivial to add in here.
    // transmitted slots per radio frame is always 15 because we do not use fractional.
    // Guess L2 doesnt care about TPC either, but whatever.
    //int mNData1;
    //int mNData2;	// not used for CCPCH, always 0.
    //int mNFbi;	// uplink dpcch only.  layer1 might be interested in this, but not layer2.
};

static int nPilot2Index(int npilot) {
    switch (npilot) {
    case 0: return 0;
    case 2: return 0;
    case 4: return 1;
    case 8: return 2;
    case 16: return 3;
    default: assert(0);
    }
}

static SlotFormat Table18(int format,int kbps,int sf, int bitsPerSlot, int data, int pilot, int tfci)
{
    SlotFormat result;
    result.mSlotFormat = format;
    //mBitRate = kbps;
    result.mSF = sf;
    result.mBitsPerSlot = bitsPerSlot;
    result.mNData1 = data;
    result.mNData2 = 0;
    result.mNPilot = pilot;
    result.mPilotIndex = nPilot2Index(result.mNPilot);
    result.mNTfci = tfci;
    result.mNTpc = 0;
    assert(result.mNData1 + result.mNData2 + result.mNPilot + result.mNTfci + result.mNTpc == result.mBitsPerSlot);
    return result;
}

// table 10: SCCPCH fields.
// We will only use one for the forseeable future.
// Note that tfci is huge.  This implies that for DCH over FACH they expect you to
// use a big TFS and assign different TrCh for each DCH.
SlotFormat SlotInfoSccpch[18] = {
        // format	kbps	sf	  b/slot  data	  pilot	  tfci
    Table18(0,		30,		256,	20,		20,		0,		0),
    Table18(1,		30,		256,	20,		12,		8,		0),
    Table18(2,		30,		256,	20,		18,		0,		2),
    Table18(3,		30,		256,	20,		10,		8,		2),
    Table18(4,		60,		128,	40,		40,		0,		0),
    Table18(5,		60,		128,	40,		32,		8,		0),
    Table18(6,		60,		128,	40,		38,		0,		2),
    Table18(7,		60,		128,	40,		30,		8,		2),
    Table18(8,		120,	 64,	80,		72,		0,		8),
    Table18(9,		120,	 64,	80,		64,		8,		8),
    Table18(10,		240,	 32,	160,	152,	0,		8),
    Table18(11,		240,	 32,	160,	144,	8,		8),
    Table18(12,		480,	 16,	320,	312,	0,		8),
    Table18(13,		480,	 16,	320,	296,	16,		8),
    Table18(14,		960,	  8,	640,	632,	0,		8),
    Table18(15,		960,	  8,	640,	616,	16,		8),
    Table18(16,		1920,	  4,	1280,	1272,	0,		8),
    Table18(17,		1920,	  4,	1280,	1256,	16,		8)
};
// These are the slot formats we (would) choose to use for downlink SCCPCH DCH at each SF,
// if we wanted to support multiple SF, which we dont.
// So this is way overkill, but I put it in for symmetric beauty with DCH, which does need it.
static const int sNumTiers = 7;	// seven tree tiers for SF=4 to SF=256.
static unsigned SlotInfoSccpchByTier[sNumTiers] = {
    16,	// SF=4
    14,	// SF=8
    12,	// SF=16
    10,	// SF=32
     8,	// SF=64
     6,	// SF=128
     2	// SF=256
    /**** 5-7-2012: Harvind says dont use pilot bits, so changed this:
    17,	// SF=4
    15,	// SF=8
    13,	// SF=16
    11,	// SF=32
     9,	// SF=64
     7,	// SF=128
     3	// SF=256
     ***/
};


unsigned sf2tier(unsigned sf)		// Return the tree tier for a given SF.
{
    // This is not particularly efficient, but I could not bring myself to call log().
    switch (sf) {
        case   4: return 0;
        case   8: return 1;
        case  16: return 2;
        case  32: return 3;
        case  64: return 4;
        case 128: return 5;
        case 256: return 6;
        default:
            LOG(ERR) << "Invalid SF:"<<sf;
            assert(0);	// Oops!
    }
}
static SlotFormat *getDlSlotFormat(PhChType chType,unsigned dlSF)
{
    int slotnum;
    switch (chType) {
    case DPDCHType:
        break;
    case SCCPCHType:
        slotnum = SlotInfoSccpchByTier[sf2tier(dlSF)];
        return &SlotInfoSccpch[slotnum];
        break;
    default: return NULL;	// Invalid phChType.
    }
}

// Same result as PhCh::getDlRadioFrameSize but if you dont have a channel pointer handy.
unsigned getDlRadioFrameSize(PhChType chtype, unsigned sf)
{
    if (chtype == PCCPCHType) {
        // Special case, 18 bits/slot = 270 bits / frame.
        // And caller must not forget to multiply by 2 because it is always TTI=20ms.
        return 270;
    }
    SlotFormat *slot = getDlSlotFormat(chtype,sf);
    return gFrameSlots * (slot->mNData1 + slot->mNData2);
}
#endif // UMTSPHCH_H
