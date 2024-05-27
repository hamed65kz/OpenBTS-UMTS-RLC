#ifndef URLC_H
#define URLC_H

#include <queue>
#include <string>
#include "shareTypes.h"
#include <vector>

class URlc{
public:
    static void initRLCConfigs();

    static void macPushUpRxCCCH(char* bit_pdu, int pdu_len); // frame + frame len
    static void macPushUpRXDCCH(char* bit_pdu, int pdu_len, RbId rbid, UeIdType id_type,int UEid); //frame + length + rb id + (urnti/crnti)

    static RlcSdu* rrcRecvDCCH(); //sdu + len + ue id + rbid
    static RlcSdu* rrcRecvCCCH();//sdu + len

    static void rrcSendDCCH(char* sdu,int sdu_len, UeIdType id_type, int UEid, RbId rbid,std::string desc);//(id = urnti/crnti)
    static void rrcSendCCCH(char* sdu,int sdu_len, std::string desc); //sdu  + len // all ccch except rrc conn setup
    static void rrcSendRRCConnectionSetup(uint32_t urnti, uint16_t crnti,char* sdu,int sdu_len); //only rrc conn set up

    static ::std::vector<RlcPdu*> macReadTx(); // pdu + pdu len + rbid + ueid(crnti/urnti) + cccd/dcch



private:
    static ::std::queue<RlcSdu*> RxCCCHQueue;

};
#endif
