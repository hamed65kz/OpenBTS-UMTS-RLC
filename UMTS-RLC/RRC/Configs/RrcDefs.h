#ifndef RRCDEFS_H
#define RRCDEFS_H

#include "../../shareTypes.h"

// Permissible to call list.erase(itr)
// Do not advance the itr inside the loop, it is done by the macro.
#define RN_FOR_ITR(type,list,itr) \
    if ((list).size()) \
		for (\
			type::iterator itr,next = (list).begin(); \
			(itr = next++) != (list).end();\
			itr=next\
				)

typedef enum CodingRate {
    CodingRate_half	= 0,
    CodingRate_third	= 1
}CodingRate;

typedef enum ChannelCodingType_PR {
    ChannelCodingType_PR_NOTHING,	/* No components present */
    ChannelCodingType_PR_noCoding,
    ChannelCodingType_PR_convolutional,
    ChannelCodingType_PR_turbo
}ChannelCodingType_PR;

enum TTICodes {
    TTI10ms=0,
    TTI20ms=1,
    TTI40ms=2,
    TTI80ms=3
};

// (pat) Physical channel types.
enum PhChType {
    CPICHType,		// Common pilot channel, carries sync, not a TrCh.
    PCCPCHType,		// Primary common control physical channel, carries beacon.
    SCCPCHType,		// carries downlink FACH (common downlink, control or data) and PCH (paging)
    PRACHType,		// carries uplink RACH
    DPDCHType 		// dedicated physical data channel, bidirectional, carries DCH.
};

class RrcDefs
{
    public:
    //enum CodingRate { CodingRate_half, CodingRate_third };	// These are from ASN
    //enum CodingType { NoCoding, Convolutional, Turbo };
    typedef ChannelCodingType_PR CodingType;	// Borrow from ASN
    static const CodingType Convolutional = ChannelCodingType_PR_convolutional;
    static const CodingType Turbo = ChannelCodingType_PR_turbo;
    static const int CodingRate_half = CodingRate::CodingRate_half;
    static const int CodingRate_third = CodingRate::CodingRate_third;
    // Max size of coded blocks from 25.212 4.2.2.2
    static const int ZConvolutional = 504;
    static const int ZTurbo = 5114;
    enum CNDomainId { UnconfiguredDomain, CSDomain, PSDomain };	// 10.3.1.1
    //enum TransportChType { DCH, RACH, FACH };	// See TrChType in UMTSCommon.h
    enum ReestablishmentTimer { useT314, useT315 };	// 10.3.3.30
    struct URNTI {
        unsigned SrncId:12;
        unsigned SRnti:20;
    };
    typedef unsigned CRNTI; // 16 bits
    static const unsigned maxRBMuxOptions = 2;
    static const unsigned maxRBperRAB = 2;	// Is this right?  What about having 3 sub-rab flows for voice?
    static const unsigned maxCtfcBits = 5;	// Up to 32; 16 is not enough for tbsize=340 at SF=4,
    static const unsigned maxTfc = (1<<maxCtfcBits);
    static const unsigned maxTfPerTrCh = maxTfc;	// Max Transport Formats per TrCh.
    static const unsigned maxTrCh = 4;		// Max TrCh per RB.
    static const unsigned maxTbPerTrCh = 32;
    static const unsigned maxTFS = maxTbPerTrCh;	// Max number of transport formats per TrCh, not per TFC.
    static const unsigned maxRBid = 32;	// From 10.3.4.16;  numbered 1..32 with 1-4 reserved for SRBs
    typedef uint32_t RbSet;	// A set of RbIds, numbered starting from 0.
    static const uint32_t RbSetDefault = 0xffffffff;	// all of them.
    static const unsigned maxTrChid = 32;	// From 10.3.5.18;  numbered 1-32.

    // Encoder/Decoder block sizes for the various encoder types.
    // 25.212 4.2.3 These are the output size [Yi] of encoding size input bits [Ki]
    // although there are special cases if Ki gets too small.
    // The inline prevents gcc from whining or emitting unused code.
    // 4.2.3 Input/Output Size of Convolutional and Turbo Coding, and I quote:
    // "Ki is the number of bits in each code block.  Yi is the number of encoded bits."
    // "convolutional coding with rate 1/2: Yi = 2*Ki + 16; rate 1/3: Yi = 3*Ki + 24;"
    // "turbo coding with rate 1/3: Yi = 3*Ki + 12."
    static unsigned R2EncodedSize(unsigned Xi, unsigned *codeInBkSz=NULL, unsigned *fillBits=NULL);   // rate 1/2
    //static unsigned R3EncodedSize(unsigned Ki);      // rate 1/3
    static unsigned TurboEncodedSize(unsigned Ki, unsigned *codeInBkSz=NULL, unsigned *fillBits=NULL); // Turbo
    static unsigned R2DecodedSize(unsigned Yi);     // rate 1/2
    //static unsigned R3DecodedSize(unsigned Yi);     // rate 1/3
    static unsigned TurboDecodedSize(unsigned Yi);  // Turbo
    static unsigned encodedSize(CodingType ct, CodingRate cr, unsigned Yi);
};


#endif // RRCDEFS_H
