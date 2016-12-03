#include <cstddef>
#include "BitTampon.hpp"
#include "QrSegment.hpp"


QAmiensRCodeGeneration::QrSegment::Mode::Mode(int mode, int cc0, int cc1, int cc2) :
		modeBits(mode) {
	numBitsCharCount[0] = cc0;
	numBitsCharCount[1] = cc1;
	numBitsCharCount[2] = cc2;
}


int QAmiensRCodeGeneration::QrSegment::Mode::numCharCountBits(int ver) const {
	if      ( 1 <= ver && ver <=  9)  return numBitsCharCount[0];
	else if (10 <= ver && ver <= 26)  return numBitsCharCount[1];
	else if (27 <= ver && ver <= 40)  return numBitsCharCount[2];
	else  throw "Version number out of range";
}


const QAmiensRCodeGeneration::QrSegment::Mode QAmiensRCodeGeneration::QrSegment::Mode::NUMERIC     (0x1, 10, 12, 14);
const QAmiensRCodeGeneration::QrSegment::Mode QAmiensRCodeGeneration::QrSegment::Mode::ALPHANUMERIC(0x2,  9, 11, 13);
const QAmiensRCodeGeneration::QrSegment::Mode QAmiensRCodeGeneration::QrSegment::Mode::BYTE        (0x4,  8, 16, 16);
const QAmiensRCodeGeneration::QrSegment::Mode QAmiensRCodeGeneration::QrSegment::Mode::KANJI       (0x8,  8, 10, 12);



QAmiensRCodeGeneration::QrSegment QAmiensRCodeGeneration::QrSegment::makeBytes(const std::vector<uint8_t> &data) {
	return QrSegment(Mode::BYTE, data.size(), data, data.size() * 8);
}


QAmiensRCodeGeneration::QrSegment QAmiensRCodeGeneration::QrSegment::makeNumeric(const char *digits) {
	BitTampon bb;
	int accumData = 0;
	int accumCount = 0;
	int charCount = 0;
	for (; *digits != '\0'; digits++, charCount++) {
		char c = *digits;
		if (c < '0' || c > '9')
			throw "String contains non-numeric characters";
		accumData = accumData * 10 + (c - '0');
		accumCount++;
		if (accumCount == 3) {
			bb.appendBits(accumData, 10);
			accumData = 0;
			accumCount = 0;
		}
	}
	if (accumCount > 0)  // 1 or 2 digits remaining
		bb.appendBits(accumData, accumCount * 3 + 1);
	return QrSegment(Mode::NUMERIC, charCount, bb.getBytes(), bb.getBitLength());
}


QAmiensRCodeGeneration::QrSegment QAmiensRCodeGeneration::QrSegment::makeAlphanumeric(const char *text) {
	BitTampon bb;
	int accumData = 0;
	int accumCount = 0;
	int charCount = 0;
	for (; *text != '\0'; text++, charCount++) {
		char c = *text;
		if (c < ' ' || c > 'Z')
			throw "String contains unencodable characters in alphanumeric mode";
		accumData = accumData * 45 + ALPHANUMERIC_ENCODING_TABLE[c - ' '];
		accumCount++;
		if (accumCount == 2) {
			bb.appendBits(accumData, 11);
			accumData = 0;
			accumCount = 0;
		}
	}
	if (accumCount > 0)  // 1 character remaining
		bb.appendBits(accumData, 6);
	return QrSegment(Mode::ALPHANUMERIC, charCount, bb.getBytes(), bb.getBitLength());
}


std::vector<QAmiensRCodeGeneration::QrSegment> QAmiensRCodeGeneration::QrSegment::makeSegments(const char *text) {
	// Select the most efficient segment encoding automatically
	std::vector<QrSegment> result;
	if (*text == '\0');  // Leave the vector empty
	else if (QrSegment::isNumeric(text))
		result.push_back(QrSegment::makeNumeric(text));
	else if (QrSegment::isAlphanumeric(text))
		result.push_back(QrSegment::makeAlphanumeric(text));
	else {
		std::vector<uint8_t> bytes;
		for (; *text != '\0'; text++)
			bytes.push_back((unsigned char &&) static_cast<uint8_t>(*text));
		result.push_back(QrSegment::makeBytes(bytes));
	}
	return result;
}


QAmiensRCodeGeneration::QrSegment::QrSegment(const Mode &md, int numCh, const std::vector<uint8_t> &b, int bitLen) :
		mode(md),
		numChars(numCh),
		data(b),
		bitLength(bitLen) {
	if (numCh < 0 || bitLen < 0 || b.size() != static_cast<unsigned int>((bitLen + 7) / 8))
		throw "Invalid value";
}


int QAmiensRCodeGeneration::QrSegment::getTotalBits(const std::vector<QrSegment> &segs, int version) {
	if (version < 1 || version > 40)
		throw "Version number out of range";
	int result = 0;
	for (size_t i = 0; i < segs.size(); i++) {
		const QrSegment &seg(segs.at(i));
		int ccbits = seg.mode.numCharCountBits(version);
		// Fail if segment length value doesn't fit in the length field's bit-width
		if (seg.numChars >= (1 << ccbits))
			return -1;
		result += 4 + ccbits + seg.bitLength;
	}
	return result;
}


bool QAmiensRCodeGeneration::QrSegment::isAlphanumeric(const char *text) {
	for (; *text != '\0'; text++) {
		char c = *text;
		if (c < ' ' || c > 'Z' || ALPHANUMERIC_ENCODING_TABLE[c - ' '] == -1)
			return false;
	}
	return true;
}


bool QAmiensRCodeGeneration::QrSegment::isNumeric(const char *text) {
	for (; *text != '\0'; text++) {
		char c = *text;
		if (c < '0' || c > '9')
			return false;
	}
	return true;
}


const int8_t QAmiensRCodeGeneration::QrSegment::ALPHANUMERIC_ENCODING_TABLE[59] = {
	// SP,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  /,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ?,  @,  // ASCII codes 32 to 64
	   36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1, -1,  // Array indices 0 to 32
	   10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,  // Array indices 33 to 58
	//  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,  P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  // ASCII codes 65 to 90
};
