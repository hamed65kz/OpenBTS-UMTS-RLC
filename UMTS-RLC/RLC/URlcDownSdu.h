#ifndef URLCDOWNSDU_H
#define URLCDOWNSDU_H
#include "../Utilities/ByteVector.h"
#include "URlcBasePdu.h"
#include "../Utilities/LinkedLists.h"
struct URlcDownSdu : public URlcBasePdu
{
	ByteVector* sduData() { return this; }
	bool mDiscarded;	// Set if one or more SDUs were discarded at this SDU position.
	bool mDiscardReq;	// Discard request from upper layer.
	unsigned mMUI;		// SDU identifier, aka Message Unit Identifier.
	string mDescr;

	URlcDownSdu* mNext;	// The SDU can be placed in a SingleLinkedList
	URlcDownSdu* next() { return mNext; }
	void setNext(URlcDownSdu* next) { mNext = next; }


	URlcDownSdu(ByteVector& wData, bool wDR, unsigned wMUI, string wDescr) :
		URlcBasePdu(wData, wDescr), mDiscarded(0), mDiscardReq(wDR),
		mMUI(wMUI), mNext(0)
	{}

	// This class is always used by pointer and manually deleted, so no copy constructor
	// is needed.
	//void free() { if (mData) { delete mData; } delete this; }
	void free() { delete this; }
	size_t size() { return mDiscarded ? 0 : ByteVector::size(); }
};

// The Uplink SDU is just a ByteVector.
typedef ByteVector URlcUpSdu;
typedef SingleLinkList<URlcDownSdu> SduList_t;
#endif