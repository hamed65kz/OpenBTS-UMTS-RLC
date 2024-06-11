#ifndef URLC_H
#define URLC_H

#include <queue>
#include <string>
#include "shareTypes.h"
#include <vector>

class URlc{
public:
    static void initRLCConfigs();

    static void macPushUpRxCCCH(char* bit_pdu, int pdu_len); 
    static void macPushUpRXDCCH(char* bit_pdu, int pdu_len, RbId rbid, UeIdType id_type,int UEid); 

    static RlcSdu* rrcRecvDCCH(); 
    static RlcSdu* rrcRecvCCCH();

    static void rrcSendDCCH(char* sdu,int sdu_len, UeIdType id_type, int UEid, RbId rbid,std::string desc);
    static void rrcSendCCCH(char* sdu,int sdu_len, std::string desc); //sdu  + len // all ccch except rrc conn setup
    static void rrcSendRRCConnectionSetup(uint32_t urnti, uint16_t crnti,char* sdu,int sdu_len); //only rrc conn set up

    static ::std::vector<RlcPdu*> macReadTx(); 
private:
    static ::std::queue<RlcSdu*> RxCCCHQueue;

};
#endif
