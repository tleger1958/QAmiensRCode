#pragma once

#include <cstdint>
#include <vector>
#include "QAmiensRSegment.hpp"


namespace QAmiensRCodeGeneration {

/*
 * Une suite de bits accessibles. Les bits sont organisés en 'gros-boutien' dans un octet.
 */
class BitTampon final {

	/*---- Champs----*/
private:

	std::vector<uint8_t> donnee;
	int bitLongueur;



	/*---- Constructeur ----*/
public:

	// Crée un tampon de bits vide (longueur 0).
	BitTampon();



	/*---- Méthodes ----*/
public:

	// Renvoie le nombre de bits dans le tampon, qui est une valeur positive ou nulle.
	int obtenirLongueurBit() const;


	// Retourne une copie de tous les octets, et remplissage jusqu'à l'octet le plus proche.
	std::vector<uint8_t> obtenirOctets() const;


	// Ajoute le nombre donné de bits de la valeur donnée à cette séquence.
	// Si 0 ≤ len ≤ 31, alors le nombre donné de bits vaut 0 ≤ val < 2^(longueur).
	void ajouterBits(uint32_t val, int longueur);


	// Ajoute les données du segment donné à ce tampon de bits.
	void ajouterDonnees(const QAmiensRSegment &seg);

};

}
