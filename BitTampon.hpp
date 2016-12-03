#pragma once

#include <cstdint>
#include <vector>
#include "QrSegment.hpp"


namespace QAmiensRCodeGeneration {

/* 
 * An appendable sequence of bits. Bits are packed in big endian within a byte.
 */
class BitTampon final {
	
	/*---- Fields ----*/
private:
	
	std::vector<uint8_t> data;
	int bitLength;
	
	
	
	/*---- Constructor ----*/
public:
	
	// Creates an empty bit buffer (length 0).
	BitTampon();
	
	
	
	/*---- Methods ----*/
public:
	
	// Returns the number of bits in the buffer, which is a non-negative value.
	int getBitLength() const;
	
	
	// Returns a copy of all bytes, padding up to the nearest byte.
	std::vector<uint8_t> getBytes() const;
	
	
	// Appends the given number of bits of the given value to this sequence.
	// If 0 <= len <= 31, then this requires 0 <= val < 2^len.
	void appendBits(uint32_t val, int len);
	
	
	// Appends the data of the given segment to this bit buffer.
	void appendData(const QrSegment &seg);
	
};

}
