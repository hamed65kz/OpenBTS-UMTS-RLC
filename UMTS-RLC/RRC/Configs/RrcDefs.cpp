#include "RrcDefs.h"
#include <assert.h>

// Encoding/decoding may require more than one code block if size > Z;
// It may not be possible to get an exact fit, which means rate matching will be required.
// Just round down to the next block size that will fit in Yi after encoding.
// This is surprisingly tricky.
unsigned RrcDefs::R2DecodedSize(unsigned Yi) // rate 1/2: base formula is: Ki = (Yi-16)/2;
{
    //R2Test();
    unsigned Zin = ZConvolutional;
    unsigned Zout = 2*Zin+16;			// encoded size of Z; this applies to each block.
    //unsigned Ci = (Yi + Zout-1)/Zout;	// number of coded blocks.
    unsigned Ci = 1 + (Yi-1)/Zout;		// number of coded blocks.
    unsigned Ki = (Yi - Ci*16)/2;		// This may be slightly too large.
    unsigned Yi2;
    while ((Yi2=R2EncodedSize(Ki)) > Yi) { Ki--; }	// Search for best.
    unsigned Ytest = R2EncodedSize(Ki);
    //printf("Yi=%d Ci=%d Ki=%d Yi2=%d Ytest=%d\n",Yi,Ci,Ki,Yi2,Ytest);
    //assert(Ytest <= Yi && Ytest >= Yi-16*Ci);
    assert(Ytest <= Yi);
    return Ki;
}


//unsigned TurboDecodedSize(unsigned Yi) { return (Yi-12)/3; }  // Turbo
unsigned RrcDefs::TurboDecodedSize(unsigned Yi) // rate 1/2: base formula is: Ki = (Yi-16)/2;
{
        //R2Test();
        unsigned Zin = ZTurbo;
        unsigned Zout = 3*Zin+12;                       // encoded size of Z; this applies to each block.
        //unsigned Ci = (Yi + Zout-1)/Zout;     // number of coded blocks.
        unsigned Ci = 1 + (Yi-1)/Zout;          // number of coded blocks.
        unsigned Ki = (Yi - Ci*12)/3;           // This may be slightly too large.
        unsigned Yi2;
        while ((Yi2=TurboEncodedSize(Ki)) > Yi) { Ki--; }  // Search for best.
        unsigned Ytest = TurboEncodedSize(Ki);
        //printf("Yi=%d Ci=%d Ki=%d Yi2=%d Ytest=%d\n",Yi,Ci,Ki,Yi2,Ytest);
        //assert(Ytest <= Yi && Ytest >= Yi-16*Ci);
        assert(Ytest <= Yi);
        return Ki;
}

// Xi is the total number of bits to encode, which will require Ci code blocks.
// rate 1/2: base formula is: YiR = 2*Ki+16
unsigned RrcDefs::R2EncodedSize(unsigned Xi, unsigned *codeBkSz, unsigned *fillBits)
{
    // From 24.212 4.2.2.2: code block segmentation.
    // The Xi may need to be encoded in several separate blocks, which may
    // require filler bits if Xi is not evenly divisible.
    int Z = ZConvolutional;
    int Ci = (Xi+Z-1)/Z;	// number of coded blocks. Ci = ceil(Xi/Z);
    int Ki = (Ci==0) ? 0 : (Xi+Ci-1)/Ci;	// number of bits per block, including filler bits. Ki = ceil(Xi/Ci)
    // If Ci=3, Xi=3,Ki=1 Xi=4,Ki=2
    if (fillBits) *fillBits = Ci * Ki - Xi;		// number of filler bits, unused here.
    if (codeBkSz) *codeBkSz = Ki;
    return Ci*(2*Ki+16);	// total number of bits, including filler bits.
}

// unsigned TurboEncodedSize(unsigned Ki) { return 3*Ki+12; }    // Turbo
unsigned RrcDefs::TurboEncodedSize(unsigned Xi, unsigned *codeBkSz, unsigned *fillBits)
{
    // From 24.212 4.2.2.2: code block segmentation.
    // The Xi may need to be encoded in several separate blocks, which may
    // require filler bits if Xi is not evenly divisible.
    int Z = ZTurbo;
    int Ci = (Xi+Z-1)/Z;    // number of coded blocks. Ci = ceil(Xi/Z);
    int Ki = (Ci==0) ? 0 : (Xi+Ci-1)/Ci;  // number of bits per block, including filler bits. Ki = ceil(Xi/Ci)
    // If Ci=3, Xi=3,Ki=1 Xi=4,Ki=2
    if (fillBits) *fillBits = Ci * Ki - Xi;             // number of filler bits, unused here.
    if (codeBkSz) *codeBkSz = Ki;		// Size of each coded block.
    return Ci*(3*Ki+12);    // total number of bits, including filler bits.
}


