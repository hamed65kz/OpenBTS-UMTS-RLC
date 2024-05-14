#ifndef BITVECTOR_H
#define BITVECTOR_H
#include "Vector.h"
#include "../shareTypes.h"

class BitVector : public Vector<char> {


public:

	/**@name Constructors. */
	//@{

	/**@name Casts of Vector constructors. */
	//@{
	BitVector(char* wData, char* wStart, char* wEnd)
		:Vector<char>(wData, wStart, wEnd)
	{
		BVDEBUG("bvc1\n");
	}
	BitVector(size_t len = 0) :Vector<char>(len) { }
	BitVector(const Vector<char>& source) :Vector<char>(source) { BVDEBUG("bvc3\n"); }
	BitVector(Vector<char>& source) :Vector<char>(source) { BVDEBUG("bvc4\n"); }
	BitVector(BitVector& src) : Vector<char>(src) { BVDEBUG("bvc5\n"); }
	// (pat) There MUST be non-inherited copy constructors in every non-trivial class
	// or you get improperly constructed objects.
	BitVector(const BitVector& src) :Vector<char>(src) { BVDEBUG("bvc6\n"); }
	BitVector(const Vector<char>& source1, const Vector<char> source2) :Vector<char>(source1, source2) { BVDEBUG("bvc7\n"); }
	//@}

	/** Construct from a string of "0" and "1". */
	BitVector(const char* valString);
	//@}

	/** Index a single bit. */
	bool bit(size_t index) const
	{
		// We put this code in .h for fast inlining.
		const char* dp = mStart + index;
		assert(dp < mEnd);
		return (*dp) & 0x01;
	}

	/**@name Casts and overrides of Vector operators. */
	//@{
	BitVector segment(size_t start, size_t span)
	{
		char* wStart = mStart + start;
		char* wEnd = wStart + span;
		assert(wEnd <= mEnd);
		return BitVector(NULL, wStart, wEnd);
	}

	BitVector alias()
	{
		return segment(0, size());
	}

	const BitVector alias() const
	{
		return segment(0, size());
	}

	const BitVector segment(size_t start, size_t span) const
	{
		return (BitVector)(Vector<char>::segment(start, span));
	}

	BitVector head(size_t span) { return segment(0, span); }
	const BitVector head(size_t span) const { return segment(0, span); }
	BitVector tail(size_t start) { return segment(start, size() - start); }
	const BitVector tail(size_t start) const { return segment(start, size() - start); }
	//@}


	void zero() { fill(0); }




		/** Invert 0<->1. */
	void invert();

	/**@name Byte-wise operations. */
	//@{
	/** Reverse an 8-bit vector. */
	void reverse8();
	/** Reverse entire vector. */
	void reverse();
	/** Reverse groups of 8 within the vector (byte reversal). */
	void LSB8MSB();
	//@}

	/**@name Serialization and deserialization. */
	//@{
	uint64_t peekField(size_t readIndex, unsigned length) const;
	uint64_t peekFieldReversed(size_t readIndex, unsigned length) const;
	uint64_t readField(size_t& readIndex, unsigned length) const;
	uint64_t readFieldReversed(size_t& readIndex, unsigned length) const;
	void fillField(size_t writeIndex, uint64_t value, unsigned length);
	void fillFieldReversed(size_t writeIndex, uint64_t value, unsigned length);
	void writeField(size_t& writeIndex, uint64_t value, unsigned length);
	void writeFieldReversed(size_t& writeIndex, uint64_t value, unsigned length);
	void write0(size_t& writeIndex) { writeField(writeIndex, 0, 1); }
	void write1(size_t& writeIndex) { writeField(writeIndex, 1, 1); }
	//@}

	/** Sum of bits. */
	unsigned sum() const;

	/** Reorder bits, dest[i] = this[map[i]]. */
	void map(const unsigned* map, size_t mapSize, BitVector& dest) const;

	/** Reorder bits, dest[map[i]] = this[i]. */
	void unmap(const unsigned* map, size_t mapSize, BitVector& dest) const;

	/** Pack into a char array. */
	void pack(unsigned char*) const;

	/** Unpack from a char array. */
	void unpack(const unsigned char*);

	/** Make a hexdump string. */
	void hex(std::ostream&) const;
	std::string hexstr() const;

	/** Unpack from a hexdump string.
	*  @returns true on success, false on error. */
	bool unhex(const char*);
	std::ostream& textBitVector(std::ostream& os) const;
	std::string str() const;

	// (pat) set() is required when the other is not a variable, for example, a function return.
	// In C++, if you pass a function return as a reference then C++ silently creates a const temp variable,
	// which in this class causes a constructor in the Vector class to allocate a new copy of the other
	// as a possibly surprising side effect.
	// If you use set() instead, you get a reference to other, instead of an allocated copy of other.
	void set(BitVector other)	// That's right.  No ampersand.
	{
		clear();
		mData = other.mData;
		mStart = other.mStart;
		mEnd = other.mEnd;
		other.mData = NULL;
	}

	bool operator==(const BitVector& other) const;
}; // class BitVector



std::ostream& operator<<(std::ostream&, const BitVector&);



#endif