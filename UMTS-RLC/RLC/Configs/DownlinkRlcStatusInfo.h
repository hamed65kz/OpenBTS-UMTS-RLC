#ifndef DRSI_H
#define DRSI_H
#include "../../shareTypes.h"

struct DownlinkRlcStatusInfo { // 10.3.4.1
    UInt_z mTimerStatusProhibit;
    Bool_z mMissingPduIndicator;
    UInt_z mTimerStatusPeriodic;
};
#endif