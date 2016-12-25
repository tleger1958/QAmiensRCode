#include <cstddef>
#include "BitTampon.hpp"
#include "QAmiensRSegment.hpp"


QAmiensRCodeGeneration::QAmiensRSegment::Mode::Mode(int mode, int indicNbCar0, int indicNbCar1, int indicNbCar2) :
		modeBits(mode) {
	indicNbCar[0] = indicNbCar0;
	indicNbCar[1] = indicNbCar1;
	indicNbCar[2] = indicNbCar2;
}


int QAmiensRCodeGeneration::QAmiensRSegment::Mode::indicNbBits(int version) const {
	if      ( 1 <= version && version <=  9)  return indicNbCar[0];
	else if (10 <= version && version <= 26)  return indicNbCar[1];
	else if (27 <= version && version <= 40)  return indicNbCar[2];
	else  throw "Wesh c'est beaucoup trop grand";
}


const QAmiensRCodeGeneration::QAmiensRSegment::Mode QAmiensRCodeGeneration::QAmiensRSegment::Mode::NUMERIQUE        (0x1, 10, 12, 14);
const QAmiensRCodeGeneration::QAmiensRSegment::Mode QAmiensRCodeGeneration::QAmiensRSegment::Mode::ALPHANUMERIQUE   (0x2,  9, 11, 13);
const QAmiensRCodeGeneration::QAmiensRSegment::Mode QAmiensRCodeGeneration::QAmiensRSegment::Mode::OCTET            (0x4,  8, 16, 16);



QAmiensRCodeGeneration::QAmiensRSegment QAmiensRCodeGeneration::QAmiensRSegment::faireOctet(const std::vector<uint8_t> &data) {
	return QAmiensRSegment(Mode::OCTET, data.size(), data, data.size() * 8);
}


QAmiensRCodeGeneration::QAmiensRSegment QAmiensRCodeGeneration::QAmiensRSegment::faireNumerique(const char *chiffre) {
	BitTampon bitTampon;
	int donneeAccumulee = 0;
	int compteurAccumule = 0;
	int compteurCaractere = 0;
	for (; *chiffre != '\0'; chiffre++, compteurCaractere++) {
		char c = *chiffre;
		if (c < '0' || c > '9') throw "Y'a des carctères qui sont pas des nombres";
		donneeAccumulee = donneeAccumulee * 10 + (c - '0');
		compteurAccumule++;
		if (compteurAccumule == 3) {
			bitTampon.ajouterBits(donneeAccumulee, 10);
			donneeAccumulee = 0;
			compteurAccumule = 0;
		}
	}
	if (compteurAccumule > 0) bitTampon.ajouterBits(donneeAccumulee, compteurAccumule * 3 + 1); // Il reste 1 ou 2 chiffres
	return QAmiensRSegment(Mode::NUMERIQUE, compteurCaractere, bitTampon.obtenirOctets(), bitTampon.obtenirLongueurBit());
}


QAmiensRCodeGeneration::QAmiensRSegment QAmiensRCodeGeneration::QAmiensRSegment::faireAlphanumerique(const char *texte) {
	BitTampon bitTampon;
	int donneeAccumulee = 0;
	int compteurAccumule = 0;
	int compteurCaractere = 0;
	for (; *texte != '\0'; texte++, compteurCaractere++) {
		char c = *texte;
		if (c < ' ' || c > 'Z') throw "Y'a des caractères qui sont pas alphanumériques";
		donneeAccumulee = donneeAccumulee * 45 + TABLE_ENCODAGE_ALPHANUMERIQUE[c - ' '];
		compteurAccumule++;
		if (compteurAccumule == 2) {
			bitTampon.ajouterBits(donneeAccumulee, 11);
			donneeAccumulee = 0;
			compteurAccumule = 0;
		}
	}
	if (compteurAccumule > 0) bitTampon.ajouterBits(donneeAccumulee, 6); // reste 1 caractère
	return QAmiensRSegment(Mode::ALPHANUMERIQUE, compteurCaractere, bitTampon.obtenirOctets(), bitTampon.obtenirLongueurBit());
}


std::vector<QAmiensRCodeGeneration::QAmiensRSegment> QAmiensRCodeGeneration::QAmiensRSegment::faireSegments(const char *texte) {
	// Sélectionne le mode d'encodage le plus optimisé
	std::vector<QAmiensRSegment> result;
	if (*texte == '\0');  // Laisse le vector vide
	else if (QAmiensRSegment::estNumerique(texte)) result.push_back(QAmiensRSegment::faireNumerique(texte));
	else if (QAmiensRSegment::estAlphanumerique(texte)) result.push_back(QAmiensRSegment::faireAlphanumerique(texte));
	else {
		std::vector<uint8_t> octets;
		for (; *texte != '\0'; texte++) octets.push_back((unsigned char &&) static_cast<uint8_t>(*texte));
		result.push_back(QAmiensRSegment::faireOctet(octets));
	}
	return result;
}


QAmiensRCodeGeneration::QAmiensRSegment::QAmiensRSegment(const Mode &md, int numCh, const std::vector<uint8_t> &b, int bitLen) :
		mode(md),
		numChars(numCh),
		donnee(b),
		bitLength(bitLen)
{
	if (numCh < 0 || bitLen < 0 || b.size() != static_cast<unsigned int>((bitLen + 7) / 8)) throw "Valeur invalide";
}


int QAmiensRCodeGeneration::QAmiensRSegment::getTotalBits(const std::vector<QAmiensRSegment> &segs, int version) {
	if (version < 1 || version > 40) throw "Version non valide";
	int result = 0;
	for (size_t i = 0; i < segs.size(); i++) {
		const QAmiensRSegment &seg(segs.at(i));
		int ccbits = seg.mode.indicNbBits(version);
		// Échoue si la valeur de longueur de segment ne correspond pas à la largeur de bit du champ de longueur
		if (seg.numChars >= (1 << ccbits)) return -1;
		result += 4 + ccbits + seg.bitLength;
	}
	return result;
}

//Teste si on peut encoder le texte en Alphanumérique
bool QAmiensRCodeGeneration::QAmiensRSegment::estAlphanumerique(const char *texte) {
	for (; *texte != '\0'; texte++) {
		char c = *texte;
		if (c < ' ' || c > 'Z' || TABLE_ENCODAGE_ALPHANUMERIQUE[c - ' '] == -1) return false;
	}
	return true;
}

//Teste si on peut encoder le texte en Numérique
bool QAmiensRCodeGeneration::QAmiensRSegment::estNumerique(const char *texte) {
	for (; *texte != '\0'; texte++) {
		char c = *texte;
		if (c < '0' || c > '9') return false;
	}
	return true;
}


const int8_t QAmiensRCodeGeneration::QAmiensRSegment::TABLE_ENCODAGE_ALPHANUMERIQUE[59] = {
	// SP,  !,  ",  #,  $,  %,  &,  ',  (,  ),  *,  +,  ,,  -,  .,  /,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  :,  ;,  <,  =,  >,  ?,  @,  // Codes ASCII de 32 à 64
	   36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1, -1,  // Indices du tableau de 0 à 32
	   10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,  // Indices du tableau de 33 à 58
	//  A,  B,  C,  D,  E,  F,  G,  H,  I,  J,  K,  L,  M,  N,  O,  P,  Q,  R,  S,  T,  U,  V,  W,  X,  Y,  Z,  // Codes ASCII de 65 à 90
}; //GG LE TABLEAU TOUT BEAU HUGO
