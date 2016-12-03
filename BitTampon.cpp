#include <cstddef>
#include "BitTampon.hpp"


QAmiensRCodeGeneration::BitTampon::BitTampon() :
	data(),
	bitLength(0) {}


int QAmiensRCodeGeneration::BitTampon::getBitLength() const {
	return bitLength;
}


std::vector<uint8_t> QAmiensRCodeGeneration::BitTampon::getBytes() const {
	return data;
}


void QAmiensRCodeGeneration::BitTampon::appendBits(uint32_t val, int len) {
	if (len < 0 || len > 32 || (len < 32 && (val >> len) != 0))
		throw "Value out of range";
	size_t newBitLen = bitLength + len;
	while (data.size() * 8 < newBitLen)
		data.push_back(0);
	for (int i = len - 1; i >= 0; i--, bitLength++)  // Append bit by bit
		data.at(bitLength >> 3) |= ((val >> i) & 1) << (7 - (bitLength & 7));
}


void QAmiensRCodeGeneration::BitTampon::appendData(const QrSegment &seg) {
	size_t newBitLen = bitLength + seg.bitLength;
	while (data.size() * 8 < newBitLen)
		data.push_back(0);
	for (int i = 0; i < seg.bitLength; i++, bitLength++) {  // Append bit by bit
		int bit = (seg.data.at(i >> 3) >> (7 - (i & 7))) & 1;
		data.at(bitLength >> 3) |= bit << (7 - (bitLength & 7));
	}
}
