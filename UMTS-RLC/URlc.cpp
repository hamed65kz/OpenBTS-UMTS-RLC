#include "URlc.h"
#include "RRC/URRC.h"
//#include <queue>
#include "RLC/Tran/URlcTransUm.h"
#include "MAC/MACEngine.h"
#include "Utilities/Logger.h"

std::queue<RlcSdu*> URlc::RxCCCHQueue ;


void URlc::initRLCConfigs()
{
	gLogInit("openbts-umts", "NOTICE", LOG_LOCAL7);
	rrcInitCommonCh(UMTS_PRACH_SF,UMTS_SCCPCH_SF);
	setupDlRlcCCCH();
}
void URlc::macPushUpRXDCCH(char* pdu, int pdu_len, RbId rbid, UeIdType id_type , int UEid){
	//URNTI = 0;
    //CRNTI = 1;
    int ueidtype = 0;// =urnti
    if(id_type == UeIdType::CRNTI)
        ueidtype =1;

	UEInfo *uep = gRrc.findUe(ueidtype, UEid);
	if (uep == NULL) {
		LOG(INFO) << "Could not find UE with id " << UEid;
		cout << " - Could not find UE with id " << UEid;
		return;
	}
	else{
		int a = 0;
	}

    BitVector dcchpdu;
    Vector<char> v1 = Vector<char>(pdu_len);
    for (int j =0; j < pdu_len; j++) {
      v1[j] = pdu[j];
    }

    dcchpdu = BitVector(v1);

    uep->ueWriteLowSide(rbid, dcchpdu, stCELL_FACH);
}

void URlc::macPushUpRxCCCH(char* pdu, int pdu_len){
    RlcSdu* sdu = new RlcSdu();
    sdu->payload = pdu;
    sdu->payload_length = pdu_len;
    sdu->rbid=-1;
    sdu->crnti=-1;
    sdu->urnti=-1;
    RxCCCHQueue.push(sdu);
}


RlcSdu* URlc::rrcRecvCCCH()
{
	if (RxCCCHQueue.size() > 0)
	{
        RlcSdu* first = RxCCCHQueue.front();
        first->isDCCH=false;
		RxCCCHQueue.pop();
        return first;
	}
	return nullptr;
}

/**
Blocking read.
@return Pointer to object (will not be NULL).
*/
RlcSdu* URlc::rrcRecvDCCH(){
	RrcUplinkMessage *msg = gRrcUplinkQueue.read();

    RlcSdu* sdu= new RlcSdu();

    sdu->payload_length = msg->msgSdu->size();
    sdu->payload = new char[sdu->payload_length]();
    memcpy(sdu->payload,msg->msgSdu->begin(),sdu->payload_length);
    sdu->payload_string = msg->msgSdu->hexstr();
    sdu->rbid = msg->msgRbid;
    sdu->urnti = msg->msgUep->mCRNTI;
    sdu->crnti = msg->msgUep->mCRNTI;
    sdu->isDCCH=true;
    msg->msgSdu->clear();
    delete msg;
    return sdu;

}
void URlc::rrcSendRRCConnectionSetup(uint32_t urnti, uint16_t crnti,char* sdu,int sdu_len)
{
    LOG(INFO) << "rrcSendRRCConnectionSetup urnti = " <<urnti<<" crnti = "<< crnti;
	//UEInfo *uep = gRrc.findUeByAsnId(&aid);
	UEInfo *uep = gRrc.findUe(false, urnti);
	if (uep == NULL) {
        uep = new UEInfo(urnti,crnti);
		//comment = "UL_CCCH_MessageType_PR_rrcConnectionRequest (new UE)";
	}
	else{
		int a = 0;
	}
	//uep->ueSetState(stIdleMode);
	uep->ueConnectRlc(gRrcDcchConfig, stCELL_FACH);	
	const std::string descrRrcConnectionSetup("RRC_Connection_Setup_Message");
    rrcSendCCCH(sdu,sdu_len, descrRrcConnectionSetup);
}
void URlc::rrcSendCCCH(char* sdu,int sdu_len, std::string desc)
{
    ByteVector ccchpdu = ByteVector(sdu, sdu_len);

    writeHighSideCcch(ccchpdu, desc);
}
void URlc::rrcSendDCCH(char* sdu,int sdu_len, UeIdType id_type, int UEid, RbId rbid, std::string desc)
{
    int ueidtype = 0;// =urnti
    if(id_type == UeIdType::CRNTI)
        ueidtype =1;
	UEInfo *uep = gRrc.findUe(ueidtype, UEid);
	if (uep == NULL) {
		LOG(INFO) << "Could not find UE with id " << UEid;
		return;
	}
    ByteVector dcchpdu = ByteVector(sdu, sdu_len);

    uep->ueWriteHighSide((RbId)rbid, dcchpdu, desc);
}
vector<RlcPdu*> URlc::macReadTx(){
    vector<RlcPdu*> dcch_pdus = flushUE();
    RlcPdu* ccch_pdu = flushQ();
	if (ccch_pdu)
	{
		dcch_pdus.push_back(ccch_pdu);
	}
	return dcch_pdus;
}
