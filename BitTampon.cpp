#include <cstddef>
#include "BitTampon.hpp"

//Constructeur
QAmiensRCodeGeneration::BitTampon::BitTampon() :
	donnee(),
	bitLongueur(0) {}


int QAmiensRCodeGeneration::BitTampon::obtenirLongueurBit() const {
	return bitLongueur;
}


std::vector<uint8_t> QAmiensRCodeGeneration::BitTampon::obtenirOctets() const {
	return donnee;
}


void QAmiensRCodeGeneration::BitTampon::ajouterBits(uint32_t val, int longueur) {
	if (longueur < 0 || longueur > 32 || (longueur < 32 && (val >> longueur) != 0)) throw "Valeur en dehors de la plage définie";
	size_t nouvelleTailleBit = bitLongueur + longueur;
	while (donnee.size() * 8 < nouvelleTailleBit) donnee.push_back(0);
	for (int i = longueur - 1; i >= 0; i--, bitLongueur++) donnee.at(bitLongueur >> 3) |= ((val >> i) & 1) << (7 - (bitLongueur & 7)); // Ajout bit par bit
}


void QAmiensRCodeGeneration::BitTampon::ajouterDonnees(const QAmiensRSegment &seg) {
	size_t nouvelleTailleBit = bitLongueur + seg.bitLength;
	while (donnee.size() * 8 < nouvelleTailleBit) donnee.push_back(0);
	for (int i = 0; i < seg.bitLength; i++, bitLongueur++) {  // Ajout bit par bit
		int bit = (seg.donnee.at(i >> 3) >> (7 - (i & 7))) & 1;
		donnee.at(bitLongueur >> 3) |= bit << (7 - (bitLongueur & 7));
	}
}
