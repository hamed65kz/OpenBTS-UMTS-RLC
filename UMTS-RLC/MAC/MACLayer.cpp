#include "MACLayer.h"
//#include "../Ue/UEInfo.h"
#include "../RRC/URRC.h"

MACLayer::MACLayer()
{

}

// Receive a RACH from the FEC via the MacSwitch.
// Decode the header and send CCCH messages to RRC directly and DCCH messages
// to their UE, which will route them to one of the RLC entities in the UE.
void MACLayer::macWriteLowSideRach(const MacTbUl&tb)
{
    // See table 9.2.1.4 "Coding of the Target Channel Type Field on RACH for FDD"
    size_t rp = 0;
    unsigned tctf = tb.readField(rp,2);
    if (tctf == 0) {
        // It is CCCH.  No other MAC fields present.  Always uses SRB0,
        // which just means the messages are for RRC itself.
        // Uses RLC TM, which just means we bypass RLC entirely.
        BitVector msg(tb.tail(rp));
        rrcRecvCcchMessage(msg,0);
    } else if (tctf == 1) {
        // It is DCCH or DTCH.  Send it off to a UE.
        unsigned ueid;
        unsigned ueidtype = tb.readField(rp,2);
        if (ueidtype == 0) {
            ueid = tb.readField(rp,32);	// URNTI
        } else if (ueidtype == 1) {
            ueid = tb.readField(rp,16);	// CRNTI
        } else {
            LOG(ERR) << "Invalid UE idtype in RACH TransportBlock";
            return;
        }
        // The ct field is the logical channel id for this transport channel,
        // but we always map it directly to RbId.
        unsigned ct = tb.readField(rp,4)+1;
        UEInfo *uep = gRrc.findUe(ueidtype,ueid);
        if (uep == NULL) {
            LOG(INFO) << "Could not find UE with id "<< ueid;
            return;
        }
        BitVector msg(tb.tail(rp));
        uep->ueWriteLowSide(ct,msg,stCELL_FACH);
    } else {
        // "PDUs with this coding will be discarded"
    }
}
