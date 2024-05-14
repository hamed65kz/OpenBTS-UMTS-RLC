#include "TransportBlock.h"
#include "../Utilities/Logger.h"

void TransportBlock::text(std::ostream&os) const
{
    os <<LOGVARM(mTime) <<LOGVARM(mScheduled) <<LOGVARM(mDescr) <<" ";
    textBitVector(os);
};


std::ostream& operator<<(std::ostream& os, const TransportBlock&tbb) { tbb.text(os); return os; }
std::ostream& operator<<(std::ostream& os, const TransportBlock*ptbb) { ptbb->text(os); return os; }
