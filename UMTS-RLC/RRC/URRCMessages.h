#ifndef ASNUEID_H
#define ASNUEID_H
#include "../shareTypes.h"
#include "../Utilities/ByteVector.h"
#include "../Ue/InitialUE-Identity.h"

typedef enum UL_DCCH_MessageType_PR {
	UL_DCCH_MessageType_PR_NOTHING,	/* No components present */
	UL_DCCH_MessageType_PR_activeSetUpdateComplete,
	UL_DCCH_MessageType_PR_activeSetUpdateFailure,
	UL_DCCH_MessageType_PR_cellChangeOrderFromUTRANFailure,
	UL_DCCH_MessageType_PR_counterCheckResponse,
	UL_DCCH_MessageType_PR_handoverToUTRANComplete,
	UL_DCCH_MessageType_PR_initialDirectTransfer,
	UL_DCCH_MessageType_PR_handoverFromUTRANFailure,
	UL_DCCH_MessageType_PR_measurementControlFailure,
	UL_DCCH_MessageType_PR_measurementReport,
	UL_DCCH_MessageType_PR_physicalChannelReconfigurationComplete,
	UL_DCCH_MessageType_PR_physicalChannelReconfigurationFailure,
	UL_DCCH_MessageType_PR_radioBearerReconfigurationComplete,
	UL_DCCH_MessageType_PR_radioBearerReconfigurationFailure,
	UL_DCCH_MessageType_PR_radioBearerReleaseComplete,
	UL_DCCH_MessageType_PR_radioBearerReleaseFailure,
	UL_DCCH_MessageType_PR_radioBearerSetupComplete,
	UL_DCCH_MessageType_PR_radioBearerSetupFailure,
	UL_DCCH_MessageType_PR_rrcConnectionReleaseComplete,
	UL_DCCH_MessageType_PR_rrcConnectionSetupComplete,
	UL_DCCH_MessageType_PR_rrcStatus,
	UL_DCCH_MessageType_PR_securityModeComplete,
	UL_DCCH_MessageType_PR_securityModeFailure,
	UL_DCCH_MessageType_PR_signallingConnectionReleaseIndication,
	UL_DCCH_MessageType_PR_transportChannelReconfigurationComplete,
	UL_DCCH_MessageType_PR_transportChannelReconfigurationFailure,
	UL_DCCH_MessageType_PR_transportFormatCombinationControlFailure,
	UL_DCCH_MessageType_PR_ueCapabilityInformation,
	UL_DCCH_MessageType_PR_uplinkDirectTransfer,
	UL_DCCH_MessageType_PR_utranMobilityInformationConfirm,
	UL_DCCH_MessageType_PR_utranMobilityInformationFailure,
	UL_DCCH_MessageType_PR_mbmsModificationRequest,
	UL_DCCH_MessageType_PR_spare1
} UL_DCCH_MessageType_PR;
/**
L3 Protocol Discriminator, GSM 04.08 10.2, GSM 04.07 11.2.3.1.1.
*/
enum L3PD {
	L3GroupCallControlPD = 0x00,
	L3BroadcastCallControlPD = 0x01,
	L3PDSS1PD = 0x02,
	L3CallControlPD = 0x03,
	L3PDSS2PD = 0x04,
	L3MobilityManagementPD = 0x05,
	L3RadioResourcePD = 0x06,
	L3GPRSMobilityManagementPD = 0x08,
	L3SMSPD = 0x09,
	L3GPRSSessionManagementPD = 0x0a,
	L3NonCallSSPD = 0x0b,
	L3LocationPD = 0x0c,
	L3ExtendedPD = 0x0e,
	L3TestProcedurePD = 0x0f,
	L3UndefinedPD = -1
};

class RrcMasterChConfig;
class UEInfo;
class PhCh;

// The CCCH messages arrive on RACH for connection establishment.
// The ASC argument is passed in the initial message,
// but we dont implement priorities so we dont care what it is.
// Another way of saying this is it comes in on RACH using SRB0.
// The only thing we need out of this message is the UE Id, which has many
// possiblities, but we dont care what they are - we are just going to retransmit
// the same information in the RRC Connection Setup message.
void rrcRecvCcchMessage(BitVector &tb, unsigned asc);
void sendRrcConnectionRelease(UEInfo *uep);
void sendRrcConnectionSetup(UEInfo *uep);
static void sendRrcConnectionReleaseCcch(int32_t urnti);
static void sendCellUpdateConfirmCcch(UEInfo *uep);

// The UE initially sends its identity in the RRC Connection Request Message.
// We dont really care what it is, we just need to copy the exact
// same UE id info into the RRC Connection Setup Message, which also assigns a U-RNTI.
// From then on, we use the U-RNTI only.
class AsnUeId {
	InitialUE_Identity_PR idType;
	public:
	ByteVector mImsi, mImei, mTmsiDS41;
	UInt32_z mMcc, mMnc;
	UInt32_z mTmsi, mPtmsi, mEsn;
	UInt16_z mLac;
	UInt8_z mRac;

	public:
	AsnUeId() { idType = InitialUE_Identity_PR::InitialUE_Identity_PR_NOTHING; }
	AsnUeId(ByteVector &wImsi) : idType(InitialUE_Identity_PR::InitialUE_Identity_PR_imsi), mImsi(wImsi) {}
	//AsnUeId(ASN::InitialUE_Identity &uid) { asnParse(uid); }
	bool RaiMatches();
	bool eql(AsnUeId &other);
	//void asnParse(ASN::InitialUE_Identity &uid);
};

void setupDlRlcCCCH(int nodeBCount);
void writeHighSideCcch(ByteVector &sdu, const string descr, int nodeBIndex);
ByteVector * macReadFromCCCH(int nodeBId);
#endif // ASNUEID_H
