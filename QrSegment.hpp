#pragma once

#include <cstdint>
#include <vector>


namespace QAmiensRCodeGeneration {

/* 
 * Represents a character string to be encoded in a QR Code symbol. Each segment has
 * a mode, and a sequence of characters that is already encoded as a sequence of bits.
 * Instances of this class are immutable.
 * This segment class imposes no length restrictions, but QR Codes have restrictions.
 * Even in the most favorable conditions, a QR Code can only hold 7089 characters of data.
 * Any segment longer than this is meaningless for the purpose of generating QR Codes.
 */
class QrSegment final {
	
	/*---- Public helper enumeration ----*/
	
	/* 
	 * The mode field of a segment. Immutable. Provides methods to retrieve closely related values.
	 */
public:
	class Mode final {
		
		/*-- Constants --*/
	public:
		
		static const Mode NUMERIC;
		static const Mode ALPHANUMERIC;
		static const Mode BYTE;
		static const Mode KANJI;
		
		
		/*-- Fields --*/
		
		/* (Package-private) An unsigned 4-bit integer value (range 0 to 15) representing the mode indicator bits for this mode object. */
	public:
		const int modeBits;
		
	private:
		int numBitsCharCount[3];
		
		
		/*-- Constructor --*/
		
	private:
		Mode(int mode, int cc0, int cc1, int cc2);
		
		
		/*-- Method --*/
		
		/* 
		 * (Package-private) Returns the bit width of the segment character count field for this mode object at the given version number.
		 */
	public:
		int numCharCountBits(int ver) const;
		
	};
	
	
	
	/*---- Public static factory functions ----*/
public:
	
	/* 
	 * Returns a segment representing the given binary data encoded in byte mode.
	 */
	static QrSegment makeBytes(const std::vector<uint8_t> &data);
	
	
	/* 
	 * Returns a segment representing the given string of decimal digits encoded in numeric mode.
	 */
	static QrSegment makeNumeric(const char *digits);
	
	
	/* 
	 * Returns a segment representing the given text string encoded in alphanumeric mode. The characters allowed are:
	 * 0 to 9, A to Z (uppercase only), space, dollar, percent, asterisk, plus, hyphen, period, slash, colon.
	 */
	static QrSegment makeAlphanumeric(const char *text);
	
	
	/* 
	 * Returns a list of zero or more segments to represent the given text string.
	 * The result may use various segment modes and switch modes to optimize the length of the bit stream.
	 */
	static std::vector<QrSegment> makeSegments(const char *text);
	
	
	/*---- Public static helper functions ----*/
public:
	
	/* 
	 * Tests whether the given string can be encoded as a segment in alphanumeric mode.
	 */
	static bool isAlphanumeric(const char *text);
	
	
	/* 
	 * Tests whether the given string can be encoded as a segment in numeric mode.
	 */
	static bool isNumeric(const char *text);
	
	
	
	/*---- Instance fields ----*/
public:
	
	/* The mode indicator for this segment. */
	const Mode mode;
	
	/* The length of this segment's unencoded data, measured in characters. Always zero or positive. */
	const int numChars;
	
	/* The bits of this segment packed into a byte array in big endian. */
	const std::vector<uint8_t> data;
	
	/* The length of this segment's encoded data, measured in bits. Satisfies ceil(bitLength / 8) = data.size(). */
	const int bitLength;
	
	
	/*---- Constructor ----*/
public:
	
	/* 
	 * Creates a new QR Code data segment with the given parameters and data.
	 */
	QrSegment(const Mode &md, int numCh, const std::vector<uint8_t> &b, int bitLen);
	
	
	// Package-private helper function.
	static int getTotalBits(const std::vector<QrSegment> &segs, int version);
	
	
	/*---- Private constant ----*/
private:
	
	/* Maps shifted ASCII codes to alphanumeric mode character codes. */
	static const int8_t ALPHANUMERIC_ENCODING_TABLE[59];
	
};

}
