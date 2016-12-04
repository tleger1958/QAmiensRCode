#pragma once

#include <cstdint>
#include <vector>


namespace QAmiensRCodeGeneration {

/*
 * Représente une chaine de caractères qui doit être codée en QAmiensRCode.
 * Chaque segment a un mode et une séquence de caractères en bits.
 * Ici on a pas de restriction de longueur mais pour es vrais QR Codes il y en a.
 * On peut mettre au max 7 089 caractères, et si c'est plus long… tant pis on coupe !
 */

class QAmiensRSegment final {

	// Le champ mode d'un segment est immuable et fournit des méthodes pour récupérer des valeurs étroitement liées.
public:
	class Mode final {

		// Constantes
	public:

		static const Mode NUMERIQUE;
		static const Mode ALPHANUMERIQUE;
		static const Mode OCTET;
		static const Mode KANJI;


		// Champs

		// Le mode est représenté par 4 bits.
	public:
		const int modeBits;

	private:
		int indicNbCar[3];


		// Constructeur

	private:
		Mode(int mode, int indicNbCar0, int indicNbCar1, int indicNbCar2);


		// Méthode

		// Renvoie la largeur de bit du champ de comptage de caractères de segment pour cet objet mode au numéro de version donné.
	public:
		int indicNbBits(int ver) const;

	};



	// Fonctions publiques
public:

	// Mode octet
	static QAmiensRSegment faireOctet(const std::vector<uint8_t> &donnee);


	// Mode numérique
	static QAmiensRSegment faireNumerique(const char *digits);


	// Mode alphanumérique
	static QAmiensRSegment makeAlphanumeric(const char *text);


	// Mode Kanji
	static std::vector<QAmiensRSegment> makeSegments(const char *text);


	// Fonctions statiques du helper
public:

	/*
	 * Tests whether the given string can be encoded as a segment in alphanumeric mode.
	 */
	static bool isAlphanumeric(const char *text);


	/*
	 * Tests whether the given string can be encoded as a segment in numeric mode.
	 */
	static bool isNumeric(const char *text);



	// Champs d'instance
public:

	/* The mode indicator for this segment. */
	const Mode mode;

	/* The length of this segment's unencoded data, measured in characters. Always zero or positive. */
	const int numChars;

	/* The bits of this segment packed into a byte array in big endian. */
	const std::vector<uint8_t> donnee;

	/* The length of this segment's encoded data, measured in bits. Satisfies ceil(bitLength / 8) = data.size(). */
	const int bitLength;


	// Constructeur
public:

	/*
	 * Creates a new QR Code data segment with the given parameters and data.
	 */
    QAmiensRSegment(const Mode &md, int numCh, const std::vector<uint8_t> &b, int bitLen);


	// Package-private helper function.
	static int getTotalBits(const std::vector<QAmiensRSegment> &segs, int version);


	/*---- Private constant ----*/
private:

	/* Maps shifted ASCII codes to alphanumeric mode character codes. */
	static const int8_t ALPHANUMERIC_ENCODING_TABLE[59];

};

}
