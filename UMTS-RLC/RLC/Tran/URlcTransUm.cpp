#include "URlcTransUm.h"

void URlcTransUm::text(std::ostream&os)
{
    URlcTrans::textTrans(os);
    URlcTransAmUm::textAmUm(os);
    os <<LOGVAR(mVTUS);
}

URlcPdu *URlcTransUm::readLowSidePdu()
{
    if (pdusFinished()) { return NULL; }
    URlcPdu *result = new URlcPdu(mConfig.mDlPduSizeBytes,this,"dl um");

    bool newSdu = false;
    bool mSduDiscarded = fillPduData(result,1,&newSdu);
    if (newSdu && mConfig.mIsSharedRlc) { mVTUS = 0; }
    result->setUmSN(mVTUS);
    incSN(mVTUS);
    if (mSduDiscarded && mConfig.mRlcDiscard.mSduDiscardMode != TransmissionRlcDiscard::NotConfigured) {
        incSN(mVTUS);	// Informs peer that a discard occurred.
    }
    RLCLOG("readLowSidePdu sizebytes=%d",result->size());
    return result;
}

