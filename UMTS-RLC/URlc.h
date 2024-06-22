#ifndef URLC_H
#define URLC_H

#include <queue>
#include <string>
#include "shareTypes.h"
#include <vector>



class URlc{
public:
    static void initRLCConfigs(int nodeBCount);

    static void macPushUpRxCCCH(char* bit_pdu, int pdu_len, int nodeBIndex);
    static void macPushUpRXDCCH(char* bit_pdu, int pdu_len, RbId rbid, UeIdType id_type,int UEid, int nodeBIndex);

    static RlcSdu* rrcRecvDCCH(); 
    static RlcSdu* rrcRecvCCCH();

    static void rrcSendDCCH(char* sdu,int sdu_len, UeIdType id_type, int UEid, RbId rbid,std::string desc, int nodeBIndex);
    static void rrcSendCCCH(char* sdu,int sdu_len, std::string desc, int nodeBIndex); //sdu  + len // all ccch except rrc conn setup
    static void rrcSendRRCConnectionSetup(uint32_t urnti, uint16_t crnti,char* sdu,int sdu_len, int nodeBIndex); //only rrc conn set up

    static ::std::vector<RlcPdu*> macReadTx(); 
private:
    static ::std::queue<RlcSdu*> RxCCCHQueue;
    static int mNodeBCount;
};
#endif
