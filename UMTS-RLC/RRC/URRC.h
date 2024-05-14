#ifndef RRC_H
#define RRC_H

#include "../Ue/UEInfo.h"
#include "UEDefs.h"
#include "../Utilities/Threads.h"
#include <list>
#include "../RRC/URRCMessages.h"
#include "../configurations.h"

//#include <time.h>


#include "Configs\RrcMasterChConfig.h"

extern RrcMasterChConfig *gRrcCcchConfig;	// Defines TrCh, SRB0 for UE in unconnected mode.
extern RrcMasterChConfig *gRrcDcchConfig;	// Defines TrCh, SRB for UE in connected mode.
extern RrcMasterChConfig *gRrcDchPSConfig;


void rrcInitCommonCh(unsigned rachSF, unsigned fachSF);


class Rrc : public UEDefs
{
    UInt32_z mRrcRNTI;	// Next ue id.

    Thread mRrcUplinkHandler;

    //TrChConfig mRachFachTrCh;
    //Tfs *getRachTfs() { rrcDoInit(); return mRachFachTrCh.ul()->getTfs(1); }
    //Tfcs *getRachTfcs() { rrcDoInit(); return mRachFachTrCh.ul()->getTfcs(); }
    //Tfs *getFachTfs() { rrcDoInit(); return mRachFachTrCh.dl()->getTfs(1); }
    //Tfcs *getFachTfcs() { rrcDoInit(); return mRachFachTrCh.dl()->getTfcs(); }


    Bool_z inited;
    public:
    void rrcDoInit() {
        if (inited) { return; }
        inited = true;
        mRrcRNTI = 0xffff&time(NULL);
    }
	void newRNTI(uint32_t *urnti, uint16_t *crnti);
    // List of UE we have heard from.
    Mutex mUEListLock;
    typedef std::list<UEInfo*> UEList_t;
    UEList_t mUEList;

    // If ueidtype is 0, look for URNTI, else CRNTI
    UEInfo *findUe(bool ueidtypeCRNTI,unsigned ueid);
    UEInfo *findUeByUrnti(uint32_t urnti) {return findUe(false,urnti);}
    UEInfo *findUeByAsnId(AsnUeId *ueid);
    void purgeUEs();
    void addUE(UEInfo *ue);

    // Dont init anything in the constructor to avoid an initialization race with UMTSConfig.
    Rrc() { mUEListLock.unlock();}


};

extern Rrc gRrc;

#define RN_UE_FOR_ALL_RLC_DOWN(uep,rbid,rlcp) \
	URlcTrans *rlcp; \
for (RbId rbid = 0; rbid <= uep->mUeMaxRlc; rbid++) \
if ((rlcp = uep->getRlcDown(rbid)))

#define RN_UE_FOR_ALL_RLC(uep,rbid,rlc) \
	URlcPair *rlc; \
for (RbId rbid = 0; rbid <= uep->mUeMaxRlc; rbid++) \
if ((rlc = uep->getRlc(rbid)))

#endif // RRC_H
