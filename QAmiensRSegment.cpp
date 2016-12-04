#include <cstddef>
#include "BitTampon.hpp"
#include "QAmiensRSegment.hpp"


QAmiensRCodeGeneration::QAmiensRSegment::Mode::Mode(int mode, int indicNbCar0, int indicNbCar1, int indicNbCar2) :
		modeBits(mode) {
	indicNbCar[0] = indicNbCar0;
	indicNbCar[1] = indicNbCar1;
	indicNbCar[2] = indicNbCar2;
}


int QAmiensRCodeGeneration::QAmiensRSegment::Mode::indicNbBits(int ver) const {
	if      ( 1 <= ver && ver <=  9)  return indicNbCar[0];
	else if (10 <= ver && ver <= 26)  return indicNbCar[1];
	else if (27 <= ver && ver <= 40)  return indicNbCar[2];
	else  throw "Wesh c'est beaucoup trop grand";
}


const QAmiensRCodeGeneration::QAmiensRSegment::Mode QAmiensRCodeGeneration::QAmiensRSegment::Mode::NUMERIQUE        (0x1, 10, 12, 14);
const QAmiensRCodeGeneration::QAmiensRSegment::Mode QAmiensRCodeGeneration::QAmiensRSegment::Mode::ALPHANUMERIQUE   (0x2,  9, 11, 13);
const QAmiensRCodeGeneration::QAmiensRSegment::Mode QAmiensRCodeGeneration::QAmiensRSegment::Mode::OCTET            (0x4,  8, 16, 16);
const QAmiensRCodeGeneration::QAmiensRSegment::Mode QAmiensRCodeGeneration::QAmiensRSegment::Mode::KANJI            (0x8,  8, 10, 12);



QAmiensRCodeGeneration::QAmiensRSegment QAmiensRCodeGeneration::QAmiensRSegment::faireOctet(const std::vector<uint8_t> &data) {
	return QAmiensRSegment(Mode::OCTET, data.size(), data, data.size() * 8);
}


QAmiensRCodeGeneration::QAmiensRSegment QAmiensRCodeGeneration::QAmiensRSegment::faireNumerique(const char *digits) {
	BitTampon bitTampon;
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
			bitTampon.ajouterBits(accumData, 10);
			accumData = 0;
			accumCount = 0;
		}
	}
	if (accumCount > 0)  // 1 or 2 digits remaining
		bitTampon.ajouterBits(accumData, accumCount * 3 + 1);
	return QAmiensRSegment(Mode::NUMERIQUE, charCount, bitTampon.obtenirOctets(), bitTampon.obtenirLongueurBit());
}


QAmiensRCodeGeneration::QAmiensRSegment QAmiensRCodeGeneration::QAmiensRSegment::makeAlphanumeric(const char *text) {
	BitTampon bitTampon;
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
			bitTampon.ajouterBits(accumData, 11);
			accumData = 0;
			accumCount = 0;
		}
	}
	if (accumCount > 0)  // 1 character remaining
		bitTampon.ajouterBits(accumData, 6);
	return QAmiensRSegment(Mode::ALPHANUMERIQUE, charCount, bitTampon.obtenirOctets(), bitTampon.obtenirLongueurBit());
}


std::vector<QAmiensRCodeGeneration::QAmiensRSegment> QAmiensRCodeGeneration::QAmiensRSegment::makeSegments(const char *text) {
	// Select the most efficient segment encoding automatically
	std::vector<QAmiensRSegment> result;
	if (*text == '\0');  // Leave the vector empty
	else if (QAmiensRSegment::isNumeric(text))
		result.push_back(QAmiensRSegment::faireNumerique(text));
	else if (QAmiensRSegment::isAlphanumeric(text))
		result.push_back(QAmiensRSegment::makeAlphanumeric(text));
	else {
		std::vector<uint8_t> bytes;
		for (; *text != '\0'; text++)
			bytes.push_back((unsigned char &&) static_cast<uint8_t>(*text));
		result.push_back(QAmiensRSegment::faireOctet(bytes));
	}
	return result;
}


QAmiensRCodeGeneration::QAmiensRSegment::QAmiensRSegment(const Mode &md, int numCh, const std::vector<uint8_t> &b, int bitLen) :
		mode(md),
		numChars(numCh),
		donnee(b),
		bitLength(bitLen) {
	if (numCh < 0 || bitLen < 0 || b.size() != static_cast<unsigned int>((bitLen + 7) / 8))
		throw "Invalid value";
}


int QAmiensRCodeGeneration::QAmiensRSegment::getTotalBits(const std::vector<QAmiensRSegment> &segs, int version) {
	if (version < 1 || version > 40)
		throw "Version number out of range";
	int result = 0;
	for (size_t i = 0; i < segs.size(); i++) {
		const QAmiensRSegment &seg(segs.at(i));
		int ccbits = seg.mode.indicNbBits(version);
		// Fail if segment length value doesn't fit in the length field's bit-width
		if (seg.numChars >= (1 << ccbits))
			return -1;
		result += 4 + ccbits + seg.bitLength;
	}
	return result;
}


bool QAmiensRCodeGeneration::QAmiensRSegment::isAlphanumeric(const char *text) {
	for (; *text != '\0'; text++) {
		char c = *text;
		if (c < ' ' || c > 'Z' || ALPHANUMERIC_ENCODING_TABLE[c - ' '] == -1)
			return false;
	}
	return true;
}


bool QAmiensRCodeGeneration::QAmiensRSegment::isNumeric(const char *text) {
	for (; *text != '\0'; text++) {
		char c = *text;
		if (c < '0' || c > '9')
			return false;
	}
	return true;
}


const int8_t QAmiensRCodeGeneration::QAmiensRSegment::ALPHANUMERIC_ENCODING_TABLE[59] = {
	// SP,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  /,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ?,  @,  // ASCII codes 32 to 64
	   36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1, -1,  // Array indices 0 to 32
	   10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,  // Array indices 33 to 58
	//  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,  P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  // ASCII codes 65 to 90
};
