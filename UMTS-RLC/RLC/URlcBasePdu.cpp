#include "URlcBasePdu.h"
URlcBasePdu::URlcBasePdu(ByteVector& other, string& wDescr) : ByteVector(other), mDescr(wDescr) {}
URlcBasePdu::URlcBasePdu(unsigned size, string& wDescr) : ByteVector(size), mDescr(wDescr) {}
URlcBasePdu::URlcBasePdu(const BitVector& bits, string& wDescr) : ByteVector(bits), mDescr(wDescr) {}
