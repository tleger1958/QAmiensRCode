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
		int indicNbBits(int version) const;

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

	// Teste si on peut encoder en alphanumérique
	static bool estAlphanumerique(const char *text);


	// Teste si on peut encoder en numérique
	static bool estNumerique(const char *text);



	// Champs d'instance
public:

	// Mode de la séquence
	const Mode mode;

	// Nombre de caractères
	const int numChars;

	// Séquence en bits
	const std::vector<uint8_t> donnee;

	// Nombre de bits
	const int bitLength;


	// Constructeur
public:

    QAmiensRSegment(const Mode &md, int numCh, const std::vector<uint8_t> &b, int bitLen);

	static int getTotalBits(const std::vector<QAmiensRSegment> &segs, int version);


	// Petite constante privée oklm
private:

	// Table ASCII des caractères numériques
	static const int8_t TABLE_ENCODAGE_ALPHANUMERIQUE[59];

};

}
