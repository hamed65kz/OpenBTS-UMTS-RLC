#ifndef SGSN_H
#define SGSN_H

#include "../shareTypes.h"

#ifndef RN_FOR_ALL
#define RN_FOR_ALL(type,list,var) \
for (type::iterator itr = (list).begin(); \
	itr == (list).end() ? 0 : ((var = *itr++), 1);)
#endif


// The data path through SGSN is different for GPRS and UMTS.
// For GPRS it includes LLC, LLE, and SNDCP.
// For UMTS it includes PDCP, which is a no-op, so it is nearly empty.
// Uplink Data Path:
//		GPRS and UMTS have significantly different uplink paradigms in that:
//		GPRS sends all packets to a single entry point in LLC, whence packets are
//			directed based on information in the packet.
//			But there is a wrinkle in that during the attach process there are multiple active TLLIs
//			(the old one the ms uses to contact us, and the new one we are assigning) and they
//			each have their own LLC state machine, but both must map to the same MS state info.
//			TODO: The above is incorrect - there should be only one set of LLC.
//		UMTS segregates data channels into RBs, so there is one entry point at the low end of the
//			sgsn for each tuple of UE-identifier + RB-id, where the UE-identifier uniquely
//			corresponds to an SgsnInfo, and RB-id is 3 is for L3 messages and 5..15 are for user data.
// 		The GPRS MS sends stuff to:
//			sgsnWriteLowSide(ByteVector &payload,SgsnInfo *si);
//		The UMTS UE sends stuff to:
//			sgsnWriteLowSide(ByteVector &payload,RB????);
// 		At the high end of the uplink path, the PdpContext sends packets to:
//			void SgsnInfo::sgsnSend2PdpLowSide(int nsapi, ByteVector &packet)
//			which calls:
//			PdpContext *pdp->pdpWriteLowSide();
// Downlink Data Path:
// 		The pdp context sends stuff to:
//			SgsnInfo *si->sgsnWriteHighSide(ByteVector &pdu,int mNSapi);
// 		At the low end of the downlink path, stuff goes to:
//			void sgsnSend2MsHighSide(ByteVector &pdu,const char *descr, int rbid);
//			which calls:
//			SgsnInfo *si->getMS()->msDownlinkQueue.write(GPRS::DownlinkQPdu *dlpdu);

// There is one SgsnInfo for each TLLI.
// This does not map one-to-one to MSInfo structs because MSInfo change TLLI when registered.
// During the attach process an MS may call in with several different TLLIs 
// and/or PTMSIs, which will create a bunch of MSInfo structs in GPRS and corresponding
// SgsnInfos here.  We dont know what MS these really are until we get an IMSI,
// or todo: see a known TMSI/PTSMI+RAI pair.
// Upon completion of a successful attach, sgsn assigns (or looks up) a new PTMSI
// for the MS, which it will use to talk back to us with yet another new TLLI based on
// the PTMSI that it sent us.
// The Session Management State is in the GmmInfo and is per-MS, not per-TLLI.
// There is a problem that during the attach procedure, uplink and downlink
// messages may be occuring simultaneously with different TLLIs,
// even though they are just one MS and we want to keep gprs straight about
// that so it does not try to start multiple TBFs for the same MS.
// This is how we resolve all that:
// The per-tlli info is in two structs: MSInfo in gprs and SgsnInfo here.
// The MSInfo has a tlli, an oldTlli, and an altTlli, see documentation at the class.
// In the sgsn, there is one SgsnInfo for each tlli.
// (It could have been organized like MSInfo, but is not just for historical reasons.)
// In the sgsn, before we have identified the MS we keep all the information
// in the SgsnInfo, which includes everything we need to remember from the
// RAUpdate or AttachRequest messages.
// After a successful AttachComplete, we will subsequently use only
// one TLLI, and therefore one SgsnInfo, for downlink messages,
// although we still accept uplink messages using the old TLLI,
// and the MS may continue to send messages using
// those old TLLIs well into the PdpContext activation procedures.
// So this process results in one SgsnInfo that is associated with the TLLI that
// finally successfully attached, one SgsnInfo for the TLLI the MS used to initiate
// the attach, and we also may end up with a bunch of ephemeral SgsnInfo
// associated with TLLIs that the MS tried but did not finish attaching.

// Note that the ephemeral SgsnInfo must send send an L3 attach message
// (to assign the local TLLI) using the immensely over-bloated LLC machinery
// to get the MS attached to a semi-permanent SgsnInfo.
// I was really tempted just to hard-code the single LLC message that is required,
// but I didn't; we allocate a whole new LLC and send the L3 message through it.
// I think after the attach process we could just get rid of the SgsnInfo, and let them
// be created anew if the MS wants to talk to us again.
// ==========
// The LLC state machine is DEFINED as being per-TLLI.
// From 3GPP 04.64 [LLC] 4.5.1: "A logical link connection is identified by a DLCI
// consisting of two identifiers: a SAPI and a TLLI."
// However, during the Attach process we change the TLLI using the procedures
// defined in 3GPP 04.64 8.3.
// The GmmInfo holds the Session Management info, and is associated with one
// and only one MS identified by IMSI.
// There are two major types of SgsnInfo:


#endif