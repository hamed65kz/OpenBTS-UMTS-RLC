#ifndef MACLAYER_H
#define MACLAYER_H
#include "TransportBlock.h"
#include "../Utilities/Logger.h"

// Uninteresting class, but the name itself is documentation about which way it is traveling.
struct MacTbUl : public TransportBlock
{
    MacTbUl(const TransportBlock &tb) : TransportBlock(tb) {}	// great language
    MacTbUl(const MacTbUl &tb) : TransportBlock(static_cast<TransportBlock>(tb)) {}
};


class MACLayer
{
public:
    MACLayer();
    void macWriteLowSideRach(const MacTbUl&tb);
};

#endif // MACLAYER_H
