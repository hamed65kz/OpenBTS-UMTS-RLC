#ifndef URLCBASEPDU_H
#define URLCBASEPDU_H
#include "../Utilities/ByteVector.h"
#include <iostream>

using namespace std;

// All purpose pdu between rlc and mac.
// Note that only TM (transparent mode) is allowed to be non-byte aligned.
class URlcBasePdu : public ByteVector
{
public:
	string mDescr;		// For debugging, description of content.
	URlcBasePdu(unsigned size, string& wDescr);
	URlcBasePdu(ByteVector& other, string& wDescr);
	URlcBasePdu(const BitVector& bits, string& wDescr);
};
#endif



