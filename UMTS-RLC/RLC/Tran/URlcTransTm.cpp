#include "URlcTransTm.h"
#include "../../Utilities/Logger.h"
unsigned URlcTransTm::rlcGetFirstPduSizeBits() {
    ScopedLock lock(mQLock);
    if (mRlcState == RLC_STOP) {return 0;}
    URlcDownSdu *sdu;
    for (sdu = mSduTxQ.front(); sdu; sdu = sdu->next()) {
        if (sdu->mDiscarded) { continue; }		// Ignoring deleted PDUs.
        return sdu->sizeBits();
    }
    return 0;
}

unsigned URlcTransTm::rlcGetBytesAvail() {
    if (mRlcState == RLC_STOP) {return 0;}
    return rlcGetSduQBytesAvail();
}
// Just turn the SDU into a PDU and send it along.
// Caller is responsible for deleting this.
URlcBasePdu *URlcTransTm::rlcReadLowSide()
{
    ScopedLock lock(mQLock);
    if (mRlcState == RLC_STOP) {return NULL;}
    if (!mSplitSdu) {
      while (URlcDownSdu *sdu = mSduTxQ.pop_front()) {
        if (sdu->mDiscarded) {
            sdu->free();
        } else {
           /* RLCLOG("readlLowSide(sizebits=%d,descr=%s,rb=%d)",
                sdu->sizeBits(), sdu->mDescr.c_str(),mrbid);*/

			char buf[300] = { 0 };
			sprintf(buf, "readlLowSide(sizebits=%d,descr=%s,rb=%d)", sdu->sizeBits(), sdu->mDescr.c_str(), mrbid);
			PATLOG(LOG_DEBUG, buf);

            return sdu;
        }
      }
    }
    return NULL;	// Shouldnt happen - MAC should check q size first.
}
