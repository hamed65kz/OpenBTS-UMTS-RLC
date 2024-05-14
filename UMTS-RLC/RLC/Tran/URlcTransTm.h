#ifndef URLCTRANSTM_H
#define URLCTRANSTM_H

#include "URlcTrans.h"

class URlcTransTm :
    public virtual URlcBase, public URlcTrans
{	public:
    URlcTransTm(RBInfo *rbInfo, UEInfo *uep) :
        URlcBase(URlcModeTm,uep,rbInfo)
        {}
    // There is only an sduq, not a pduq, and reading a pdu comes straight from the sduq.
    void rlcPullLowSide(unsigned amt) {}
    unsigned rlcGetPduCnt() { return getSduCnt(); } //HKZ : ? in TM we have segmentation,so  pdu is not equall to sdu
    unsigned rlcGetFirstPduSizeBits();
    unsigned rlcGetBytesAvail();

    // Just turn the SDU into a PDU and send it along.
    URlcBasePdu *rlcReadLowSide();//HKZ : Transmit down to MAC, MAC read PDU from RLC.
    void text(std::ostream &os) { textTrans(os); }
    bool pdusFinished() { assert(mSplitSdu == NULL); return getSduCnt() == 0; }//HKZ : rlcReadLowSide & GetDataPdu(in TrasnAM) use it for check pdu existance in queue
};

#endif