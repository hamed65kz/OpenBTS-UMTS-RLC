#include "URlcRecv.h"
#include "../RrcUplinkMessage.h"
#include "../../Utilities/Logger.h"

// Send an sdu out the high side of the rlc, routed based on rbid.
void URlcRecv::rlcSendHighSide(URlcUpSdu *sdu)
{
    if (this->mHighSideFunc) {
        // This is used for testing.
        this->mHighSideFunc(*sdu,mrbid);
        delete sdu;
        return;
    }
    switch (this->mrbid) {
    case 0:
        // Probably RBInfo misconfiguration.
        // 12-29-2012 pat: This assertion is occurring; I think maybe we are deleting an AM-RLC
        // while it is still in use by the uplink.
        // The SRB0 messages were supposed to go to the RLC in the MAC-C.
        RLCERR("invalid RLC high side message to SRB0, probably RB mis-configuration");
        assert(0);
    case 1: case 2: case 3:
        // case 1 and 2 are messages to RRC.
        // case 3 is a Layer3 message, but handled the same to crack it out of its RRC message wrapper.
        gRrcUplinkQueue.write(new RrcUplinkMessage(sdu,mUep,mrbid));
		printf("%s\n", ">RLC send SDU to RRC");
        break;
    default:
        if (mrbid >= 16) {
            // RLC mis-configuration.
            assert(0);
        }
        // User data.
        gSgsnUplinkQueue.write(new RrcUplinkMessage(sdu,mUep,mrbid));
        break;
    }
}

