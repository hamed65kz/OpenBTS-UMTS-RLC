#ifndef SHARET
#define SHARET
#include <stdint.h>
#include <iostream>	// For size_t

enum URlcMode {
	URlcModeTm, URlcModeUm, URlcModeAm
};
typedef unsigned RbId;		// Numbered starting at 0 for SRB0, so SRB1 is 1.
typedef unsigned TrChId;	// Numbered starting at 0 herein.
typedef unsigned TfcId;		// TFC (Transport Format Combination) id, numbered 0 .. 
typedef unsigned TfIndex;	// TF index, specifies which TF in a TFS for a single TrCh.

// Since the initial RRC connection setup is on SRB0, and subsequently we use SRB2
// for everything, one wonders what would ever be send on SRB1?
// It appears to be used for the HS-DSCH channels that use a bunch of extra
// junk inside MAC that has a hard time getting synchronized.
const RbId SRB0 = 0;
const RbId SRB1 = 1;
const RbId SRB2 = 2;
const RbId SRB3 = 3;
const RbId SRB4 = 4;
enum UeIdType{
    URNTI,
    CRNTI
};

class RlcSdu{
  public:
    char* payload;
    int payload_length;
    int rbid;
    int crnti;
    int urnti;
	int nodeBId;
    std::string payload_string;
    bool isDCCH;
};

typedef RlcSdu RlcPdu;
// We dont bother to define *= /= etc.; you'll have to convert: a*=b; to: a=a*b;
#define _INITIALIZED_SCALAR_FUNCS(Classname,Basetype,Init) \
	Classname() : value(Init) {} \
	Classname(Basetype wvalue) { value = wvalue; } /* Can set from basetype. */ \
	operator Basetype(void) const { return value; }		/* Converts from basetype. */ \
	Basetype operator++() { return ++value; } \
	Basetype operator++(int) { return value++; } \
	Basetype operator=(Basetype wvalue) { return value = wvalue; } \
	Basetype operator+=(Basetype wvalue) { return value = value + wvalue; } \
	Basetype operator-=(Basetype wvalue) { return value = value - wvalue; } \
	Basetype* operator&() { return &value; } \

#define _DECLARE_SCALAR_TYPE(Classname_i,Classname_z,Basetype) \
	template <Basetype Init> \
	struct Classname_i { \
		Basetype value; \
		_INITIALIZED_SCALAR_FUNCS(Classname_i,Basetype,Init) \
	}; \
	typedef Classname_i<0> Classname_z;


// Usage:
// Where 'classname' is one of the types listed below, then:
// 		classname_z specifies a zero initialized type;
// 		classname_i<value> initializes the type to the specified value.
// We also define Float_z.
_DECLARE_SCALAR_TYPE(Int_i, 	Int_z,  	int)
_DECLARE_SCALAR_TYPE(Char_i,	Char_z, 	signed char)
_DECLARE_SCALAR_TYPE(Int16_i,	Int16_z,	int16_t)
_DECLARE_SCALAR_TYPE(Int32_i,	Int32_z,	int32_t)
_DECLARE_SCALAR_TYPE(UInt_i,  	UInt_z,  	unsigned)
_DECLARE_SCALAR_TYPE(UChar_i,  	UChar_z,  	unsigned char)
_DECLARE_SCALAR_TYPE(UInt8_i,  	UInt8_z,  	uint8_t)
_DECLARE_SCALAR_TYPE(UInt16_i,	UInt16_z,	uint16_t)
_DECLARE_SCALAR_TYPE(UInt32_i,	UInt32_z,	uint32_t)
_DECLARE_SCALAR_TYPE(Size_t_i,	Size_t_z,	size_t)
_DECLARE_SCALAR_TYPE(Bool_i,  	Bool_z, 	bool)

// float is special, because C++ does not permit the template initalization:
struct Float_z {
    float value;
    _INITIALIZED_SCALAR_FUNCS(Float_z,float,0)
};
struct Double_z {
    double value;
    _INITIALIZED_SCALAR_FUNCS(Double_z,double,0)
};


static const int AmSNS = 4096;		// 12 bits wide
static const int UmSNS = 128;		// 7 bits wide


typedef Int_z URlcSN;	// A modulo mSNS number.

class ItemWithValueAndWidth {
public:
	virtual unsigned getValue() const = 0;
	virtual unsigned getWidth() const = 0;
};

#define UMTS_RLC_TransmissionBufferSize 1000000 //Buffer size in Bytes for RLC Transmission layer.

typedef signed char		int8_t;
typedef unsigned char		uint8_t;
typedef short			int16_t;
typedef unsigned short		uint16_t;
typedef int			int32_t;
typedef unsigned		uint32_t;
//typedef long long		int64_t;
//typedef unsigned long long	uint64_t;


#endif

