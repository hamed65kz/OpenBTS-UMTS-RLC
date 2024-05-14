#ifndef URLCAM_H
#define URLCAM_H
#include "Recv/URlcRecvAm.h"
#include "Tran/URlcTransAm.h"
#include "URlcPdu.h"

//class URlcRecvAm;
//class URlcTransAm;//HKZ

class URlcAm :  public URlcRecvAm , public URlcTransAm  // UMTS RLC Acknowledged Mode Entity
{
    friend class URlcRecvAm;
    friend class URlcTransAm;

    void recvResetPdu(URlcPdu*pdu);
    void recvResetAck(URlcPdu*pdu);
    unsigned getRlcHeaderSize() { return 2; }

    URlcConfigAm mConfig;	// The one and only config for the entire class hierarchy.

    // Need a mutex only for RLC-AM.  The MAC pulls data from the transmitter
    // in one thread, and the receiver may be driven asynchronously
    // from the FEC classes by another thread, and status pdus may cause
    // activity in both transmitter and receiver, so need a lock.
    Mutex mAmLock;
    string mAmid;
    const char *rlcid() { return mAmid.c_str(); }

    public:
    URlcAm(RBInfo *rbInfo,RrcTfs *dltfs,UEInfo *uep,unsigned dlPduSize);
    // See 9.2.1.7 and 9.2.2.14
    // HFN defined in 25.331 8.5.8 - 8.5.10, for RRC Message Integrity Protection.
    UInt_z mULHFN;	// Security stuff.
    UInt_z mDLHFN;

    //URlcTransAm *transmitter() { return static_cast<URlcTransAm*>(this); }
    //URlcRecvAm *receiver() { return static_cast<URlcRecvAm*>(this); }

};
#endif