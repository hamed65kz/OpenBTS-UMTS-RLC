#ifndef URLCRECVTM_H
#define URLCRECVTM_H

#include "URlcRecv.h"


class URlcRecvTm :
    public virtual URlcBase, public URlcRecv
{	public:
    URlcRecvTm(RBInfo *rbInfo, UEInfo *uep);
    void rlcWriteLowSide(const BitVector &pdu);
    void text(std::ostream &os) {}
};
#endif