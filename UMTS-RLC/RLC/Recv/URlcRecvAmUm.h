#ifndef URLCRECVAMUM_H
#define URLCRECVAMUM_H

#include "URlcRecv.h"
#include "../URlcPdu.h"
#include "../Configs/URlcConfigAmUm.h"


class URlcRecvAmUm : // Receive common to AM and UM modes.
    public URlcRecv, public virtual URlcBase
{

protected:

    URlcConfigAmUm *mConfig;
    URlcUpSdu *mUpSdu;	// Partial SDU being assembled, or NULL.
    void sendSdu();						// Enqueue a completed SDU.

    bool mLostPdu;	// This is UM only, but easier to put in this class.
                    // It is set only in UM mode to indicate a lost pdu,
                    // so we have to continue to discard until we find the start of a new sdu.


    void addUpSdu(ByteVector &payload);	// Add to partially assembled SDU.
    void discardPartialSdu();
    void ChopOneByteOffSdu(ByteVector &payload);
    void parsePduData(URlcPdu &pdu, int headersize, bool Eindicator, bool statusOnly);

    void recvDoReset() { discardPartialSdu(); mLostPdu = false; }

    public:
    // This class is not allocated alone; it is part of URlcRecvAm or URlcRecvUm.
    URlcRecvAmUm(URlcConfigAmUm *wConfig) : mConfig(wConfig), mUpSdu(0) {}
    URlcRecvAmUm(): mUpSdu(0) {}
    void textAmUm(std::ostream &os);
};
#endif