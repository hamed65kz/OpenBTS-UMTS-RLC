#include "MSUEAdapter.h"
#include "../configurations.h"


#define RN_UMTS 1

// This is only externally visible for UMTS because in GPRS we have
// to send messages through LLC first, so this call is made by LLC to the SGSN.
