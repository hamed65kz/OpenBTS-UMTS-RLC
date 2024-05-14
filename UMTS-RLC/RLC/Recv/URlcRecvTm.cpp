#include "URlcRecvTm.h"

URlcRecvTm::URlcRecvTm(RBInfo *rbInfo, UEInfo *uep) :
        URlcBase(URlcModeTm,uep,rbInfo)
        {}
    // TM Messages just pass through.
void URlcRecvTm::rlcWriteLowSide(const BitVector &pdu) {
        URlcUpSdu *sdu = new ByteVector((pdu.size() + 7)/8);
        pdu.pack(sdu->begin());
        rlcSendHighSide(sdu);
}
