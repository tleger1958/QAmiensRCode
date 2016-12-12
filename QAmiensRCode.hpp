#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "QAmiensRSegment.hpp"

#include <SFML/Graphics.hpp>


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
	class NivCorrErr final {
		// Constantes déclarées par ordre croissant de protection contre les erreurs.
	public:
		const static NivCorrErr BAS, MOYEN, MOYEN_PLUS, HAUT;

	public:
		const int ordinal;  // (Public) Dans la plage 0 à 3 (entier non signé de 2 bits).
		const int formatBits;  // Dans la plage 0 à 3 (entier non signé de 2 bits).

		// Constructeur.
	private:
		NivCorrErr(int ord, int fb);
	};



	/*---- Fonctions statiques publique d'usine :D (si vous avez pas compris la blague, MAAALHEEUUR A VOUS !) ----*/
public:

	/*
	 * Renvoie un QAmiensRCode qui représente la chaîne de texte Unicode donnée, au niveau de correction d'erreur donné.
	 * En tant que limite supérieure conservatrice, cette fonction est garantie de réussite pour les chaînes qui ont 738 ou
	 * moins de points de code Unicode (ça n'est pas unités de code UTF-16). La version de QAmiensRCode la plus petite
	 * possible est automatiquement choisie pour la sortie.
	 * Le niveau de correction d'erreurs du résultat peut être supérieur à l'argument 'nivCorrErreur' si cela peut être fait
	 * sans augmenter la version.
	 */
	static QAmiensRCode encoderTexte(const char *texte, const NivCorrErr &nivCorrErreur);


	/*
     * Renvoie un QAmiensRCode qui représente la chaîne de données binaires donnée, au niveau de correction d'erreur donné.
     * Cette fonction encode toujours en utilisant le mode de segment binaire, pas n'importe quel mode texte. Le nombre maximal
     * d'octets autorisés est 2953. La version de code qamiensrcode la plus petite possible est automatiquement choisie pour la sortie.
     * Le niveau NivCorrErr du résultat peut être supérieur à l'argument nivCorrErreur si cela peut être fait sans augmenter la version.
	 */
	static QAmiensRCode encoderOctet(const std::vector<uint8_t> &donnee, const NivCorrErr &nivCorrErreur);


	/*
     * Renvoie un QAmiensRCode qui représente les segments de données spécifiés, avec les paramètres de codage spécifiés.
     * La version de code qamiensrcode la plus petite possible dans la plage spécifiée est automatiquement choisie pour la sortie.
     * Cette fonction permet à l'utilisateur de créer une séquence personnalisée de segments qui commute
     * entre les modes (tels que alphanumérique et binaire) pour coder le texte plus efficacement.
     * Cette fonction est considérée comme étant un niveau plus bas que le simple encodage de texte ou de données binaires.
	 */
	static QAmiensRCode encoderSegments(const std::vector<QAmiensRSegment> &segs, const NivCorrErr &nivCorrErreur,
		int minVersion=1, int maxVersion=40, int masque=-1, bool optimiserCorrection=true);  // Des paramètres optionnels



	/*---- Champs d'instance ----*/

	// Paramètres scalaires publics immuables
public:

	// Version comprise entre 1 et 40
	const int version;

	// La hauteur et la largeur se mesurent en modules. C'est toujours égal à la version × 4 + 17, donc ça va de 21 à 177.
	const int taille;

	// Niveau de correction utilisé
	const NivCorrErr &niveauCorrectionErreur;

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
     * et le numéro de masque. C'est un constructeur de faible niveau encombrant qu'on doit pas invoquer directement.
     * Pour aller à un niveau plus haut, faut utiliser la fonction encodeSegments() (c'est trivial).
	 */
	QAmiensRCode(int ver, const NivCorrErr &nivCorrErreur, const std::vector<uint8_t> &MotscodeDonnees, int masque);


	/*
	 * Crée un nouveau QAmiensRCode à partir d'un objet existant, mais avec un modèle de masque
     * potentiellement différent. La version, le niveau de correction d'erreur, les mots de code, etc.
     * de l'objet créé sont tous identiques à l'objet argument ; seul le masque peut différer.
	 */
	QAmiensRCode(const QAmiensRCode &qamiensrcode, int masque);



	/*---- Méthodes d'instance publique ----*/
public:

	int getMasque() const;


	/*
	 * Renvoie la couleur du pixel aux coordonnées données, 0 pour blanc et 1 pour noir. (0, 0) c'est en haut à gauche.
	 * Si on est en dehors du QAmiensRCode, alors ça renvoie 0.
	 */
	int getModule(int x, int y) const;


	/*
	 * En fonction du nombre donné de modules de bordure à ajouter comme remplissage, cela renvoie une
     * chaîne dont le contenu représente un fichier XML encodant le QAmiensRCode en SVG.
     * Les saut de lignes style Unix (\n) sont toujours utilisées, sur tous les OS.
	 */
	std::string encoderSVG(int bordure) const;

	/*
	 * Renvoie une texture SFML qui conient le QAmiensRCode. Crée un tableau avec tous les pixels et les met dans une texture.
	 */
	sf::Texture encoderSFML(int bordure) const;

	/*
	 * Donne le scale pour changer la taille du QAmiensRCode pour qu'il soit bien grand.
     */
    int getEchelle(int bordure);

	/*---- Méthodes de DESSIN privées pour le constructeur ----*/
private:

	void dessinerMotifsFonction();


	// Dessine deux copies des bits de format (avec son propre code de correction d'erreur)
    // basé sur le masque donné et sur le champ de correction d'erreur de cet objet.
	void dessinerBitsFormat(int masque);


	// Dessine deux copies des bits de version (avec son propre code de correction d'erreur),
    // basé sur le champ de version de cet objet (ça s'utilise seulement pour les versions entre 7 et 40).
	void dessinerVersion();


	// Dessine un motif de viseur de format 9*9, avec le séparateur de bordure compris, avec le module central en (x, y).
	void dessinerMotifViseur(int x, int y);


	// Dessine un motif d'alignement de format 5*5, avec le module central en (x, y).
	void dessinerMotifAlignement(int x, int y);


	// Définit la couleur d'un module et le marque comme un module de fonction.
	// C'est utilisé uniquement par le constructeur.
	void definirModuleFonction(int x, int y, bool estNoir);


	/*---- Méthodes de MASQUAGE et de MOTS CODE privées pour le constructeur ----*/
private:

	// Renvoie une nouvelle chaîne d'octets qui représente les données données (lol) avec les mots-clés de correction
	// d'erreur appropriés qui y sont ajoutés, en fonction de la version de cet objet et du niveau de correction d'erreur.
	std::vector<uint8_t> ajouterCorrectionErreur(const std::vector<uint8_t> &donnees) const;


	// Dessine la séquence donnée de mots de code à 8 bits (données et correction d'erreurs) sur la zone de données entière
	// du QAmiensRCode. Les modules de fonction doivent être marqués avant d'être appelés.
	void dessinerMotsCles(const std::vector<uint8_t> &donnees);


	// Fais un XOR sur les modules de données du QAmiensRCode avec le modèle de masque donné.
	// Et à cause du XOR, appeler deux fois la fonction avec la même valeur ne change rien
	// Du coup on peut appliquer un masque, de le défaire et en essayer un autre.
	// Pour un QAmiensRCode bien formé on a besoin d'exactement un masque.
	void appliquerMasque(int masque);


	// A messy helper function for the constructors. This qamiensrcode Code must be in an unmasked state when this
	// method is called. The given argument is the requested mask, which is -1 for auto or 0 to 7 for fixed.
	// This method applies and returns the actual mask chosen, from 0 to 7.

	// C'est une fonction d'aide désordonnée pour les constructeurs. Le QAmiensRCode doit être non masqué lorsqu'on appelle cette méthode.
	// L'argument donné c'est le masque demandé, c'est -1 pour auto ou entre 0 et 7 pour fixe. Cette méthode s'applique et renvoie le masque réel choisi, de 0 à 7.
	int gererMasquageConstructeur(int masque);


	// Calcule et retourne le score de pénalité en fonction de l'état des modules actuels du QAmiensRCode.
	// C'est utilisé par l'algorithme de choix de masque automatique pour trouver le motif de masque qui donne le score le plus bas.
	int getScorePenalite() const;



	/*---- Fonctions statiques privées ----*/
private:

	// Renvoie un ensemble de positions des motifs d'alignement dans l'ordre croissant. Ces positions sont utilisées sur les axes x et y.
	// Chaque valeur du tableau résultant est dans l'intervalle [0, 177]. Cette fonction pourrait être implémentée sous forme
	// de tableau de 40 listes de longueur variable d'octets non signés.
	static std::vector<int> getPosMotifAlignement(int version);


	// Renvoie le nombre de modules de données brutes (bits) disponibles au numéro de version donné.
	// Ces modules de données sont utilisés à la fois pour les mots de code de données d'utilisateur et les mots de code de correction d'erreur.
	// Cette fonction pourrait être mise en œuvre en tant que table de consultation de 40 entrées.
	static int getNbModulesDonnesBrutes(int version);


	// Renvoie le nombre de mots de code de 8 bits (càd sans correction d'erreur) contenus dans un QAmiensRCode du numéro de version
	// et du niveau de correction d'erreur donnés, les bits de reste étant ignorés. Cette fonction pure apatride pourrait être implémentée
	// sous la forme d'une table de recherche de cellules (40 * 4).
	static int getNbMotsCode(int version, const NivCorrErr &nivCorrErreur);


	/*---- Private tables of constants ----*/
private:

	// For use in getPenaltyScore(), when evaluating which mask is best.
	static const int PENALITE_N1;
	static const int PENALITE_N2;
	static const int PENALITE_N3;
	static const int PENALITE_N4;

	static const int16_t NB_MOTSCODE_CORRECTION_ERREUR[4][41];
	static const int8_t NB_BLOCS_CORRECTION_ERREUR[4][41];



	/*---- Classe membre privée ----*/
private:

	/*
	 * Calcule les mots de code de correction d'erreur Reed-Solomon pour une séquence de
	 * mots de code de données à un degré donné. Les objets sont immuables, et l'état ne dépend
	 * que du degré. Cette classe existe parce que le polynôme du diviseur n'a pas besoin d'être recalculé pour chaque entrée.
	 */
	class GenerateurReedSolomon final {

		/*-- Champ immuable --*/
	private:

		// Coefficients du polynôme du diviseur, mémorisé de la puissance la plus élevée à la plus basse, à l'exclusion du premier terme qui est toujours 1.
		// Par exemple, le polynôme x ^ 3 + 255x ^ 2 + 8x + 93 est stocké sous la forme uint8 {255, 8, 93} .
		std::vector<uint8_t> coefficients;


		/*-- Constructeur --*/
	public:

		/*
		 * Crée un générateur d'NivCorrErr Reed-Solomon pour le degré donné.
		 * Cela pourrait être implémenté comme une table de
		 * consultation sur toutes les valeurs de paramètres possibles, au lieu d'être un algorithme.
		 */
		GenerateurReedSolomon(int degre);


		/*-- Méthode --*/
	public:

		/*
		 * Calcule et renvoie l'NivCorrErr Reed-Solomon pour la séquence de données... donnée !
		 * L'objet retourné est toujours un nouveau tableau d'octets. Cette méthode ne change pas l'état de l'objet.
		 */
		std::vector<uint8_t> getReste(const std::vector<uint8_t> &donnees) const;


		/*-- Fonction statique --*/
	private:

		// Renvoie le produit des deux éléments de champ donné modulo GF (2 ^ 8 / 0x11D).
		// Les arguments et le résultat sont des entiers non signés de 8 bits.
		// Cela pourrait être implémenté comme une table de consultation de 256 * 256 entrées de uint8.
		static uint8_t multiplierCommeUnCommuniste(uint8_t x, uint8_t y);

	};

};

}
