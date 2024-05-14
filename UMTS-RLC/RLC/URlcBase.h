#ifndef URLCBASE_H
#define URLCBASE_H
#include "../shareTypes.h"

#include "../Ue/RBInfo.h"
#include "../Utilities/ByteVector.h"

#include <assert.h>

typedef void (*URlcHighSideFuncType)(ByteVector &sdu, RbId rbid);

class UEInfo; // for avoid circular dependency

class URlcBase
{
	// 25.322 9.7.6 RLC Stop mode.
	// The RLC_STOP mode is supposed to block the RLC at the low end, both incoming and outgoing,
	// but continues to accept SDUs at the high end.
	// This functionality is needed because of the dorky way they do the cell state transition
	// from CELL_FACH to CELL_DCH.
	// I only implemented outgoing, then stopped - stopping incoming would need to a special queue
	// and doesnt make any sense any way becaues the corresponding RLC entity in the UE
	// has already changed its state, so we need to process the incoming PDUs.
public:
	enum RlcState {
		RLC_RUN,
		RLC_STOP,
		RLC_SUSPEND	// Not supported.
	} mRlcState;

	void rlcStop() { mRlcState = RLC_STOP; }
	void rlcResume() { mRlcState = RLC_RUN; }

	enum SufiType {	// Used for AM only.
		SUFI_NO_MORE = 0,
		SUFI_WINDOW = 1,
		SUFI_ACK = 2,
		SUFI_LIST = 3,
		SUFI_BITMAP = 4,
		SUFI_RLIST = 5,
		SUFI_MRW = 6, // Note: we dont need to use the MRW, implementing reset is sufficient.
		SUFI_MRW_ACK = 7,
		SUFI_POLL = 8
	};
	enum PduType {	// Used for AM only.
		PDUTYPE_STATUS = 0,
		PDUTYPE_RESET = 1,
		PDUTYPE_RESET_ACK = 2
	};

	URlcMode mRlcMode;

	unsigned mSNS;	// The Sequence Number Space for this RLC entity.

	UEInfo* mUep;	// The UE that owns us.  May be NULL for RLC for CCCH .
	RbId mrbid;	// The RadioBearer that we were created for.

	URlcBase(URlcMode wRlcMode, UEInfo* wUep, RBInfo* wRbp);

	// This initializer should never be called because we are a virtual class,
	// so only the most most derived (aka final) class needs an initializer,
	// and they are always provided.
	URlcBase() { assert(0); }

	// Modulo mSNS arithmetic functions.
	int deltaSN(URlcSN sn1, URlcSN sn2);
	URlcSN addSN(URlcSN sn1, URlcSN sn2);
	URlcSN minSN(URlcSN sn1, URlcSN sn2);
	URlcSN maxSN(URlcSN sn1, URlcSN sn2);
	void incSN(URlcSN& psn);
	// Currently this is used only for debug messages:
	virtual unsigned getRlcHeaderSize() { return 0; }	// If not over-ridden, return 0.
};


#endif