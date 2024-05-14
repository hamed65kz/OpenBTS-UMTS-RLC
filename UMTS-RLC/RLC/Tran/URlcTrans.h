#ifndef URLCTRANS_H
#define URLCTRANS_H

#include "../URlcBase.h"
#include "../URlcDownSdu.h"
#include <iostream>
#include "../../Utilities/Interthread.h"

using namespace std;
// Any mode transmitter
class URlcTrans : public virtual URlcBase
{
protected:
    unsigned getSduCnt();
    URlcDownSdu* mSplitSdu;
    // The SduTxQ consists of the complete vectors in the list, plus mSplitSdu,
    // which is used only for UM and AM modes to save the SDU currently being processed.
    Mutex mQLock;		// Lock for SduTxQ.
    SduList_t mSduTxQ;
    unsigned rlcGetSduQBytesAvail();





	// mVTSDU incremeted for every SDU transmission when the first SDU segment
	// is scheduled to be transmitted the first time.
	// When == Poll_SDU, send a poll and set this to 0.
	// Used when for Am mode "poll every Poll_SDU" is configured.
	unsigned mVTSDU;

	// TODO: This count will be off if an sdu was deleted, because
	// am empty place-holder is left in the sduq.  Not worth worrying about now.

    virtual bool pdusFinished() = 0; // rlcReadLowSide & GetDataPdu(in TrasnAM) use it for check pdu existance in queue


	// If exceeded we have to throw away some SDUs.
	// Where is this in the spec?
	unsigned mTransmissionBufferSizeBytes;
	string mRlcid;

public:
	URlcTrans();

	virtual unsigned rlcGetBytesAvail() = 0;

	// Higher layer sends something to RLC. Same function for all modes:
	// put in the queue, but check for overflow.
	void rlcWriteHighSide(ByteVector& sdu, bool DR, unsigned MUI, string descr);//HKZ : Trasnmit Down From RRC to RLC, RRC pass SDU to RLC. All Derived classes use this parent class

	// The mutex lock for both of these is in URlcTransAm::readLowSidePdu()
    virtual void rlcPullLowSide(unsigned amt) = 0;
    virtual URlcBasePdu* rlcReadLowSide() = 0;//Transmit down to MAC, MAC read PDU from RLC. Derieved classes (TM, AMUM) have thier own impl

	// There is no guarantee that all the PDUs are the same size when in TM.
	// These functions that interact only with the mPduOutQ do not need
	// further mutex protection beyond what the Q provides.
	virtual unsigned rlcGetPduCnt() = 0;
	virtual unsigned rlcGetFirstPduSizeBits() = 0;
	virtual unsigned rlcGetDlPduSizeBytes() { return 0; }	// Not defined for RLC-TM, so return 0.

	virtual void triggerReset() { }
	void textTrans(std::ostream& os);
	const char* rlcid() { return mRlcid.c_str(); }
	virtual void text(std::ostream& os) = 0;
};
#endif