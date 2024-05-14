#ifndef URLC_H
#define URLC_H

#include "RLC/Recv/URlcRecv.h"
#include "RLC/Tran/URlcTrans.h"
#include <queue>
#include "RRC/URRCMessages.h"
#include <string>

class URlc{
public:

	static void macPushUpRxCCCH(BitVector* pdu);
	static void macPushUpRXDCCH(BitVector pdu, RbId rbid, int UEid);

	static RrcUplinkMessage* rrcRecvDCCH();
	static BitVector* rrcRecvCCCH();

	static void rrcSendDCCH(ByteVector sdu, int UEid, RbId rbid,std::string desc);
	static void rrcSendCCCH(ByteVector sdu, std::string desc);
	static void rrcSendRRCConnectionSetup(int urnti ,ByteVector sdu);

	static vector<ByteVector*> macReadTx();

	static void initRLCConfigs();
	static std::queue<BitVector*> RxCCCHQueue;

};
#endif