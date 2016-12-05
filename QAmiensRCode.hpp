#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "QAmiensRSegment.hpp"


namespace QAmiensRCodeGeneration {


/*
 * Représente une grille carrée immuable de cellules noires et blanches pour un symbole de QAmiensRCode, et
 * fournit des fonctions statiques pour créer un QAmiensRCode à partir de données textuelles ou binaires fournies par l'utilisateur.
 * Cette classe couvre la spécification du modèle 2 de QAmiensRCode, prenant en charge toutes les versions (les tailles)
 * de 1 à 40, les 4 niveaux de correction d'erreur, mais seulement 3 modes d'encodage de caractères.
 */
class QAmiensRCode final {

	/*---- Énumération publique du helper ----*/
public:

	/*
	 * Représente le niveau de correction d'erreur utilisé dans un symbole de QAmiensRCode.
	 */
	class Ecc final {
		// Constantes déclarées par ordre croissant de protection contre les erreurs.
	public:
		const static Ecc BAS, MOYEN, MOYEN_PLUS, HAUT;

	public:
		const int ordinal;  // (Public) Dans la plage 0 à 3 (entier non signé de 2 bits).
		const int formatBits;  // Dans la plage 0 à 3 (entier non signé de 2 bits).

		// Constructeur.
	private:
		Ecc(int ord, int fb);
	};



	/*---- Fonctions statiques publique d'usine :D (si vous avez pas compris la blague, MAAALHEEUUR A VOUS !) ----*/
public:

	/*
	 * Renvoie un QAmiensRCode qui représente la chaîne de texte Unicode donnée, au niveau de correction d'erreur donné.
	 * En tant que limite supérieure conservatrice, cette fonction est garantie pour réussir pour les chaînes qui ont 738 ou
	 * moins de points de code Unicode (ça n'est pas unités de code UTF-16). La version de QAmiensRCode la plus petite
	 * possible est automatiquement choisie pour la sortie.
	 * Le niveau de correction d'erreurs du résultat peut être supérieur à l'argument 'nivCorrErreur' si cela peut être fait*
	 * sans augmenter la version.
	 */
	static QAmiensRCode encoderTexte(const char *texte, const Ecc &nivCorrErreur);


	/*
     * Renvoie un QAmiensRCode qui représente la chaîne de données binaires donnée, au niveau de correction d'erreur donné.
     * Cette fonction encode toujours en utilisant le mode de segment binaire, pas n'importe quel mode texte. Le nombre maximal de
     * Les octets autorisés sont 2953. La version de code QR la plus petite possible est automatiquement choisie pour la sortie.
     * Le niveau ECC du résultat peut être supérieur à l'argument nivCorrErreur si cela peut être fait sans augmenter la version.
	 */
	static QAmiensRCode encoderOctet(const std::vector<uint8_t> &donnee, const Ecc &nivCorrErreur);


	/*
     * Renvoie un QAmiensRCode qui représente les segments de données spécifiés, avec les paramètres de codage spécifiés.
     * La version de code QR la plus petite possible dans la plage spécifiée est automatiquement choisie pour la sortie.
     * Cette fonction permet à l'utilisateur de créer une séquence personnalisée de segments qui commute
     * Entre les modes (tels que alphanumérique et binaire) pour coder le texte plus efficacement.
     * Cette fonction est considérée comme étant un niveau plus bas que le simple encodage de texte ou de données binaires.
	 */
	static QAmiensRCode encoderSegments(const std::vector<QAmiensRSegment> &segs, const Ecc &nivCorrErreur,
		int minVersion=1, int maxVersion=40, int mask=-1, bool optimiserCorrection=true);  // Des paramètres optionnels



	/*---- Champs d'instance ----*/

	// Paramètres scalaires publics immuables
public:

	// Version comprise entre 1 et 40
	const int version;

	// La hauteur et la largeur se mesurent en modules. C'est toujours égal à la version × 4 + 17, donc ça va de 21 à 177.
	const int taille;

	// Niveau de correction utilisé
	const Ecc &niveauCorrectionErreur;

    /* Modèle de masque utilisé dans ce QAmiensRCode, compris entre 0 et 7 (càd un entier de 3 bits non signé).
     * Et même si un constructeur a été appelé avec masquage automatique (Masque = -1),
     * l'objet aura toujours une valeur de masque entre 0 et 7. */
private:
	int masque;

	// Grilles de modules / pixels
private:
	std::vector<std::vector<bool>> modules;     // Le module du QAmiensRCode (false = blanc, true = noir)
	std::vector<std::vector<bool>> estFonction;  // Indique les modules fonctions, qui ne vont pas être "masqués"



	/*---- Constructeurs ----*/
public:

	/*
	 * Crée un nouveau QAmiensRCode avec la version, le niveau de correction d'erreur, le tableau de données binaires,
     * et numéro de masque. C'est un constructeur de faible niveau encombrant qu'on doit pas invoquer directement.
     * Pour aller à un niveau plus haut, faut utiliser la fonction encodeSegments().
	 */
	QAmiensRCode(int ver, const Ecc &nivCorrErreur, const std::vector<uint8_t> &dataCodewords, int masque);


	/*
	 * Crée un nouveau QAmiensRCode à partir d'un objet existant, mais avec un modèle de masque
     * potentiellement différent. La version, le niveau de correction d'erreur, les mots de code, etc.
     * de l'objet créé sont tous identiques à l'objet argument; seul le masque peut différer.
	 */
	QAmiensRCode(const QAmiensRCode &qr, int masque);



	/*---- Méthodes d'instance publique ----*/
public:

	int getMasque() const;


	/*
	 * Renvoie la couleur du pixel aux coordonnées données, 0 pour blanc et 1 pour noir. (0, 0) c'est en haut à gauche.
	 * Si on est en dehors du QAmiensRCode, alors ça renvoie 0.
	 */
	int getModule(int x, int y) const;


	/*
	 * En fonction du nombre donné de modules de bordure à ajouter comme remplissage, cela renvoie un
     * Chaîne dont le contenu représente un fichier XML SVG qui représente le QAmiensRCode.
     * Les saut de lignes style Unix (\n) sont toujours utilisées, sur tous les OS.
	 */
	std::string toSvgString(int bordure) const;



	/*---- Méthodes d'aide privée pour le constructeur: modules de fonctions de dessin ----*/
private:

	void drawFunctionPatterns();


	// Draws two copies of the format bits (with its own error correction code)
	// based on the given mask and this object's error correction level field.
	void drawFormatBits(int masque);


	// Draws two copies of the version bits (with its own error correction code),
	// based on this object's version field (which only has an effect for 7 <= version <= 40).
	void drawVersion();


	// Draws a 9*9 finder pattern including the border separator, with the center module at (x, y).
	void drawFinderPattern(int x, int y);


	// Draws a 5*5 alignment pattern, with the center module at (x, y).
	void drawAlignmentPattern(int x, int y);


	// Sets the color of a module and marks it as a function module.
	// Only used by the constructor. Coordinates must be in range.
	void setFunctionModule(int x, int y, bool isBlack);


	/*---- Private helper methods for constructor: Codewords and masking ----*/
private:

	// Returns a new byte string representing the given data with the appropriate error correction
	// codewords appended to it, based on this object's version and error correction level.
	std::vector<uint8_t> appendErrorCorrection(const std::vector<uint8_t> &data) const;


	// Draws the given sequence of 8-bit codewords (data and error correction) onto the entire
	// data area of this QR Code symbol. Function modules need to be marked off before this is called.
	void drawCodewords(const std::vector<uint8_t> &data);


	// XORs the data modules in this QR Code with the given mask pattern. Due to XOR's mathematical
	// properties, calling applyMask(m) twice with the same value is equivalent to no change at all.
	// This means it is possible to apply a mask, undo it, and try another mask. Note that a final
	// well-formed QR Code symbol needs exactly one mask applied (not zero, not two, etc.).
	void applyMask(int masque);


	// A messy helper function for the constructors. This QR Code must be in an unmasked state when this
	// method is called. The given argument is the requested mask, which is -1 for auto or 0 to 7 for fixed.
	// This method applies and returns the actual mask chosen, from 0 to 7.
	int handleConstructorMasking(int masque);


	// Calculates and returns the penalty score based on state of this QR Code's current modules.
	// This is used by the automatic mask choice algorithm to find the mask pattern that yields the lowest score.
	int getPenaltyScore() const;



	/*---- Private static helper functions ----*/
private:

	// Returns a set of positions of the alignment patterns in ascending order. These positions are
	// used on both the x and y axes. Each value in the resulting array is in the range [0, 177).
	// This stateless pure function could be implemented as table of 40 variable-length lists of unsigned bytes.
	static std::vector<int> getAlignmentPatternPositions(int ver);


	// Returns the number of raw data modules (bits) available at the given version number.
	// These data modules are used for both user data codewords and error correction codewords.
	// This stateless pure function could be implemented as a 40-entry lookup table.
	static int getNumRawDataModules(int ver);


	// Returns the number of 8-bit data (i.e. not error correction) codewords contained in any
	// QR Code of the given version number and error correction level, with remainder bits discarded.
	// This stateless pure function could be implemented as a (40*4)-cell lookup table.
	static int getNumDataCodewords(int ver, const Ecc &nivCorrErreur);


	/*---- Private tables of constants ----*/
private:

	// For use in getPenaltyScore(), when evaluating which mask is best.
	static const int PENALTY_N1;
	static const int PENALTY_N2;
	static const int PENALTY_N3;
	static const int PENALTY_N4;

	static const int16_t NUM_ERROR_CORRECTION_CODEWORDS[4][41];
	static const int8_t NUM_ERROR_CORRECTION_BLOCKS[4][41];



	/*---- Private helper class ----*/
private:

	/*
	 * Computes the Reed-Solomon error correction codewords for a sequence of data codewords
	 * at a given degree. Objects are immutable, and the state only depends on the degree.
	 * This class exists because the divisor polynomial does not need to be recalculated for every input.
	 */
	class ReedSolomonGenerator final {

		/*-- Immutable field --*/
	private:

		// Coefficients of the divisor polynomial, stored from highest to lowest power, excluding the leading term which
		// is always 1. For example the polynomial x^3 + 255x^2 + 8x + 93 is stored as the uint8 array {255, 8, 93}.
		std::vector<uint8_t> coefficients;


		/*-- Constructor --*/
	public:

		/*
		 * Creates a Reed-Solomon ECC generator for the given degree. This could be implemented
		 * as a lookup table over all possible parameter values, instead of as an algorithm.
		 */
		ReedSolomonGenerator(int degree);


		/*-- Method --*/
	public:

		/*
		 * Computes and returns the Reed-Solomon error correction codewords for the given sequence of data codewords.
		 * The returned object is always a new byte array. This method does not alter this object's state (because it is immutable).
		 */
		std::vector<uint8_t> getRemainder(const std::vector<uint8_t> &data) const;


		/*-- Static function --*/
	private:

		// Returns the product of the two given field elements modulo GF(2^8/0x11D). The arguments and result
		// are unsigned 8-bit integers. This could be implemented as a lookup table of 256*256 entries of uint8.
		static uint8_t multiply(uint8_t x, uint8_t y);

	};

};

}
