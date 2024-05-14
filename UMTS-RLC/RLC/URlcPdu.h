#ifndef URLCPDU_H
#define URLCPDU_H
#include "URlcBasePdu.h"
#include "URlcBase.h"
#include "../Utilities/LinkedLists.h"
#include "../Utilities/Interthread.h"
// May be any mode, and for AM, may be data or control pdu.
// PDU for UM and AM mode that use a common transmitter for reasons documented above,
// so we use the same PDU also.
// UM PDU:
//		SN:7;		// Used for reassembly
//		E:1
//		LI:7 or 15;
//		E:1;	// Meaning configured by upper layers.  Can be "normal" (below) or
//				// Alternative: 0 => next field is a complete SDU which
//				// is not segmented concatenated or padded
//				// 1=> next field is length indicator+E bit.
//		...
//		Data
//		Optional PAD
//class URlcPdu : public ByteVector, public Text2Str, public MemCheckURlcPdu

class URlcPdu : public URlcBasePdu
{
public:
	URlcBase* mOwner;	// The TM, UM or AM entity that owns this pdu.

	// For UM and AM data pdus:
	unsigned mPaddingStart;	// Location of padding or 0, used for piggybacked status.
	unsigned mPaddingLILocation;	// Location of LI indicator for padding, or 0.

	unsigned mVTDAT;	// For AM only, how many times PDU has been scheduled.
	bool mNacked;		// true if pdu has been negatively acknowledged.

	// Fields so this can be placed in a SingleLinkList:
	URlcPdu* mNext;	// The SDU can be placed in a SingleLinkedList
	URlcPdu* next() { return mNext; }
	void setNext(URlcPdu* next) { mNext = next; }

	URlcPdu(unsigned wSize, URlcBase* wOwner, string wDescr);
	URlcPdu(const BitVector& bits, URlcBase* wOwner, string wDescr);
	explicit URlcPdu(URlcPdu* other);

	// UM PDU fields:
	void setUmE(unsigned ebit) { setBit(7, ebit); }
	unsigned getUmE() const { return getBit(7); }
	void setUmSN(unsigned sn) { setField(0, sn, 7); }
	unsigned getUmSN() const { return getField(0, 7); }

	// AM PDU fields:
	void setAmSN(unsigned sn) { setField2(0, 1, sn, 12); }
	unsigned getAmSN() const { return getField2(0, 1, 12); }
	void setAmDC(bool isData) { setField2(0, 0, isData, 1); }
	int getAmDC() const { return getBit(0); }
	static const int sPollBit = 13;
	void setAmP(bool P) { setField2(1, (sPollBit - 8), P, 1); }
	unsigned getAmP(bool P) const { return getField2(1, (sPollBit - 8), 1); }
	unsigned getAmHE() const { return getField2(1, 6, 2); }	// but only the bottom bit of HE is used.
	void setAmHE(unsigned HE) { setField2(1, 6, HE, 2); }

	// Common functions:
	enum URlcMode rlcMode() const { return mOwner->mRlcMode; }
	void setSN(unsigned sn) { if (rlcMode() == URlcModeUm) setUmSN(sn); else setAmSN(sn); }
	unsigned getSN() const { return (rlcMode() == URlcModeUm) ? getUmSN() : getAmSN(); }
	void setEIndicator(bool ebit) { if (rlcMode() == URlcModeUm) setUmE(ebit); else setAmHE(ebit); }
	bool getEIndicator() const { return (rlcMode() == URlcModeUm) ? getUmE() : getAmHE(); }

	// For UM and AM data pdus:
	// E==1 implies there is a following LI+E (opposite of GPRS E definition.)
	void appendLIandE(unsigned licnt, unsigned E, unsigned lisize);

	// For debugging:
	unsigned getPayloadSize() const {
		return (mPaddingStart ? mPaddingStart : size()) - mOwner->getRlcHeaderSize();
	}
	void text(std::ostream& os) const;
};
typedef InterthreadQueue<URlcPdu, SingleLinkList<URlcPdu> > PduList_t;

//class URlcPduUm : public URlcPdu
//{	public:
//	URlcPduUm(unsigned wsize,unsigned wlisize) : URlcPdu(wsize,wlisize) {}
//	void setSN(unsigned sn) { setField(0,sn,7); }
//	unsigned getSN() { return getField(0,7); }
//};

// AM PDU:
//		DC:1;	1 for data pdu, 0 for control pdu
//		SN:12;		// Used for retransmission.
//		P:1;		// Polling bit: 1=> request a status report.
//		HE:2;		// Header Extension Type.
//					// 0 => succeeding octet contains data
//					// 1 => succeeding octet contains LI+E
//					// 2 => succeeding octet contains data and the last octet
//					//	of the PDU is last octet of SDU, but only if 
//					// "Use special value of HE field" is configured.
//					// 3 => reserved.
//		LI:7 or 15;	// Has special encoding, see 9.2.2.8
//		E:1;	// Always "Normal" E-bit interpretation, whcih is:
//				// 0 => next field is data, status or padding;
//				// 1=> next field is another length indicator + E bit.
//		...
//		Data
// 		Optional Piggybacked STATUS PDU.
//		Optional PAD
//class URlcPduAm : public URlcPdu
//{
//	void setSN(unsigned sn) { setField2(0,1,sn,12); }
//	unsigned getSN() { return getField2(0,1,12); }
//	void setDC(bool isData) { setField2(0,0,isData,1); }
//	void setP(bool P) { setField2(0,5,P,1); }
//	unsigned getP(bool P) { return getField2(0,5,1); }
//	unsigned getHE(unsigned HE) { return getField2(0,6,2); }
//	void setHE(unsigned HE) { setField2(0,6,HE,2); }
//	void setE(bool val) { setHE(val ? 1 : 0); }
//};

// AM Status PDU:  Size bounded by maximum RLC PDU size used by the logical channel.
//		DC:1;	0 for control pdu
//		PDUType:3;	// 0 = STATUS pdu type
//		SUFI(1) 	//SUFI defined 9.2.2.11
//		...
//		SUFI(k)
//		PAD	 to byte boundary, or if fixed size logical channel, to that PDU size.
//	RESET and RESET ACK PDU:
//		DC:1;
//		PDUType:3; 	//1 = RESET, 2 = RESET ACK
//		RSN:1;		// 9.2.2.13.  Dont understand. Indicates RESET is resent?
//		R1:3;		// Byte alignment, always 0.
//		HFNI:20;		// Indicates the Hyper Frame Number?
//		PAD

#endif
