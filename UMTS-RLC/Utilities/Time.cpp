#include "Time.h"

int16_t FNDelta(int16_t v1, int16_t v2)
{
	static const int16_t halfModulus = gHyperframe / 2;
	int16_t delta = v1 - v2;
	if (delta >= halfModulus) delta -= gHyperframe;
	else if (delta<-halfModulus) delta += gHyperframe;
	return (int16_t)delta;
}

int FNCompare(int16_t v1, int16_t v2)
{
	int16_t delta = FNDelta(v1, v2);
	if (delta>0) return 1;
	if (delta<0) return -1;
	return 0;
}
std::ostream& operator<<(std::ostream& os, const Time& ts){
	return os; //HKZ
};