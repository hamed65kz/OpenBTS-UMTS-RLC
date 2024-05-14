#ifndef URLCCONFIGUM_H
#define URLCCONFIGUM_H

#include "URlcConfigAmUm.h"

// This contains both uplink and downlink config, although it is possible
// to use one without the other.
struct URlcConfigUm : public /*virtual*/ URlcConfigAmUm
{
    //unsigned largestUlUmdPduSize // 9.2.2.8  Computed from Transport Set  See mPduSize.
#if RLC_OUT_OF_SEQ_OPTIONS
    bool mOSD;	// Out of Sequence delivery.	// 11.2.3
    bool mOSR;	// Out of Sequence reception.	// 11.2.3.1
#endif
    //bool mUseOSD;	// UM downlink only, use out-of-sequence-sdu-delivery
    //unsigned mOSD_Window_Size;	// UM downlink only, only if UseOSD
    //Z100 mTimerOSD 	// UM downlink only, only if UseOSD, to delete stored PDUs see 11.2.3.2
    //bool SN_Delivery;	// REL-7 and up.  Used in OSD mode to indicate SDU SN to higher layers.


    // I dont understand why there are both Configured_Rx_Window_Size and OSD_Window_Size.
    // I think they are the same thing.
    //unsigned mConfigured_Rx_Window_Size;	// In UM only needed if UseOSD.
    //unsigned mOSD_Window_Size;	// UM downlink only, only if UseOSD.
    //bool mUseDAR;		// UM uplink only.
    //unsigned mDAR_Window_Size;  // UM uplink only, only if DAR.
    //Z100 mTimerDAR;  // UM uplink only, only if DAR.
    URlcConfigUm(URlcInfo &rlcInfo, RrcTfs &dltfs, unsigned dlPduSize) :
        URlcConfigAmUm(URlcModeUm, rlcInfo, dltfs, dlPduSize)
        // More To Do
    {}
};
#endif