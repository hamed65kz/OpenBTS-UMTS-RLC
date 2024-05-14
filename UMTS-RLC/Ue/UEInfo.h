#ifndef UEINFO_H
#define UEINFO_H

#include "MSUEAdapter.h"

using namespace std;



class UEInfo;
// When we send an RB setup message of some kind, we save the transaction here
// so that we know what to do when we receive the message reply.
// This is necessary for RadioBearerSetup because the UE may request multiple
// setups simultaneously, eg, might ask for two IP addresses at the same time,
// and we need to keep the responses straight.
struct UeTransaction : UEDefs
{
    TransType mTransactionType;	// Event type.
    UEState mNewState;	// If not idle, new state when transaction complete.
    unsigned mRabMask;	// The mask of RBs affected by this transaction.
    uint8_t mTransactionId;
    Timeval mTransTime;	// When the event occurred.

    UeTransaction() : mTransactionType(ttComplete), mNewState(stUnused) {}
    // The rabMask is just extra info saved with the transaction to be used
    // by the transaction response; it is currently a mask of the RABs affected
    // by a radiobearer modification, but conceptually it could be arbitrary info.
    UeTransaction(UEInfo* uep, TransType type, unsigned rabMask, unsigned transactionId,
        UEState nextState = stUnused);
    void transClose() { mTransactionType = ttComplete; }
    long elapsed();	// How long since the transaction, in ms?

    const char* name() { return TransType2Name(mTransactionType); }
    std::string str();
};

class UEInfo : MSUEAdapter
{
public:
	UEInfo(AsnUeId *wUid);
	UEInfo(uint32_t urnti);
	void _initUEInfo();

    int mUeDebugId;
    //bool mStateChange;
    uint32_t mURNTI; 	// Note: The mURNTI is 12 bits SRNC id + 20 bits UE id.
    uint16_t mCRNTI;	// Used by mac on the phy interface.
    UEState mUeState;

	AsnUeId mUid;			// Self-inits.
	// There are two sets of RLCs - explanation in the big comment above.
	// We use the rbid directly for the rlc index, so SRB1 uses 1, etc.
	// They wont all be used simultaneously, but its just easier.
	// The SRB0 is never used because that rlc is in the Mac-c.
	URlcPair *mRlcsCF[gsMaxRB];		// The RLCs used in CELL_FACH state.
	URlcPair *mRlcsCDch[gsMaxRB];	// The RLCs used in CELL_DCH state.

	// TODO: Decrement mUeMaxRlc when deleting RBs.
	UInt_z mUeMaxRlc;	// Max index of any allocated rlc for this ue.

    void reestablishRlcs(){}
    string ueid() const;	// A human readable name for the UE.
    //UEState ueGetState() const { return mUeState; }

    // 25.331 13.4.27 - same TRANSACTIONS table in RRC and UE:
    //UeTransaction mTransactions[UEDefs::sMaxTransaction];
	//UeTransaction *getLastTransaction();

	Timeval mHelloTime;
	Timeval mActivityTime;	// When was last activity?
	void ueWriteLowSide(RbId rbid, const BitVector &pdu, UEState state);
	void ueRegisterActivity();

	// The external interface is by rbid - do not access the rlcs directly because
	// they may get destroyed when the UE switches states.
	// The rlc-down is always picked based on the UE state.
	// In uplink we need to be able to receive messages from both states simultaneously
	// during the cell state transition, so the MAC must specify the state.
	URlcPair *getRlc(RbId rbid);
	URlcTrans *getRlcDown(RbId rbid);
	URlcRecv *getRlcUp(RbId rbid, UEState state);
	//UeTransaction* getTransaction(unsigned transId, UEDefs::TransType ttype, const char *help);
	//void ueSetState(UEState newState);
	void ueDisconnectRlc(UEState state);
	void ueConnectRlc(RrcMasterChConfig *config, UEState nextState);
	void ueWriteHighSide(RbId rbid, ByteVector &sdu, string descr);
	unsigned newTransactionId();
	void uePullLowSide(unsigned amt);
	void ueRecvDcchMessage(ByteVector &bv, unsigned rbNum);
	void ueRecvL3Msg(ByteVector &msgframe, UEInfo *uep);
protected:

	UInt_z mNextTransactionId;	// Next Transaction Id.
	void ueRecvRrcConnectionSetupResponse(unsigned transId, bool success, const char *msgname);

};
std::ostream& operator<<(std::ostream& os, const UEInfo*uep);


#endif //UEINFO_H
