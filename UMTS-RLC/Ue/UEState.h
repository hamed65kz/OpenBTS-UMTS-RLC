#ifndef UESTATE_H
#define UESTATE_H
enum UEState {
    stUnused,
    stIdleMode,
    //stCELL_FACHPending,
    stCELL_FACH,
    //stCELL_DCHPending,
    stCELL_DCH,
    stCELL_PCH,	// we dont use thse yet
    stURA_PCH
};
#endif // UESTATE_H
