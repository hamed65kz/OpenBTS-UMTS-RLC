#include "URlcRecvUm.h"

void URlcRecvUm::text(std::ostream &os)
{
    os <<LOGVAR(mVRUS);
#if RLC_OUT_OF_SEQ_OPTIONS
    os <<LOGVAR(VRUDR) <<LOGVAR(VRUDH) <<LOGVAR(VRUDT) <<LOGVAR(VRUOH)
    <<LOGVAR2("VRUM",VRUM());
#endif
}


// Out of Sequence SDU Delivery not implemented.
// We would only need this if we used multiple physical channels, and we wont.
void URlcRecvUm::rlcWriteLowSide(const BitVector &pdubits)
{
    URlcPdu pdu(pdubits,this,"ul um");

    URlcSN sn = pdu.getSN();
#if RLC_OUT_OF_SEQ_OPTIONS	// not fully implemented
    if (mConfig->mmConfigOSR) {
        // 11.2.3.1 SDU discard and re-assembly
        if (deltaSN(sn,VRUM()) >= 0) { /*delete pdu;*/ return; }
    }
#endif
    //cout << "sn: " << sn << ", mVRUS: " << mVRUS << endl;
    if (sn != mVRUS) {
        // Lost one or more pdus.
        // Discard any partially assembled SDU.
        discardPartialSdu();
        // Set mLostPdu to continue to discard data until we find a certain start of a new sdu.
        mLostPdu = true;
    }
    mVRUS = addSN(sn,1);

    // Note: payload does not 'own' memory; must delete original pdu when finished.
    parsePduData(pdu, 1, pdu.getUmE(),false);
    // Automatic deletion of ByteVector in pdu.
}
