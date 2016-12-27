#include <algorithm>
#include <climits>
#include <cmath>
#include <iostream>
#include <cstddef>
#include <sstream>
#include "BitTampon.hpp"
#include "QAmiensRCode.hpp"


QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::NivCorrErr(int ord, int fb) :
	ordinal(ord),
	formatBits(fb) {}


const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::BAS          (0, 1);
const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::MOYEN        (1, 0);
const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::MOYEN_PLUS   (2, 3);
const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::HAUT         (3, 2);


QAmiensRCodeGeneration::QAmiensRCode QAmiensRCodeGeneration::QAmiensRCode::encoderTexte(const char *texte, const NivCorrErr &nivCorrErreur) {
	std::vector<QAmiensRSegment> segs(QAmiensRSegment::faireSegments(texte));
	return encoderSegments(segs, nivCorrErreur);
}


QAmiensRCodeGeneration::QAmiensRCode QAmiensRCodeGeneration::QAmiensRCode::encoderOctet(const std::vector<uint8_t> &donnee, const NivCorrErr &nivCorrErreur) {
	std::vector<QAmiensRSegment> segs;
	segs.push_back(QAmiensRSegment::faireOctet(donnee));
	return encoderSegments(segs, nivCorrErreur);
}


QAmiensRCodeGeneration::QAmiensRCode QAmiensRCodeGeneration::QAmiensRCode::encoderSegments(const std::vector<QAmiensRSegment> &segs, const NivCorrErr &nivCorrErreur,
    int minVersion, int maxVersion, int masque, bool optimiserCorrection) {
	if (!(1 <= minVersion && minVersion <= maxVersion && maxVersion <= 40) || masque < -1 || masque > 7) throw "Valeur invalide";

	// Détermination de la plus petite version à utiliser
	int version, donnees_bits_utilises;
	for (version = minVersion; ; version++) {
		int capacite_donnees_bits = getNbMotsCode(version, nivCorrErreur) * 8;  // Nombre de bits nécessaire
		donnees_bits_utilises = QAmiensRSegment::getTotalBits(segs, version);
		if (donnees_bits_utilises != -1 && donnees_bits_utilises <= capacite_donnees_bits) break;  // Cette version est jugé appropriée
		if (version >= maxVersion) throw "Trop de donnees"; // Toutes les versions de la gamme ne peuvent pas correspondre aux données... données ! ^^
	}

	// Augmente le niveau de correction d'erreur mais laisse les données dans la version actuelle
	const NivCorrErr *nouveauNivCorrErr = &nivCorrErreur;
	if (optimiserCorrection) {
		if (donnees_bits_utilises <= getNbMotsCode(version, NivCorrErr::MOYEN  ) * 8)  nouveauNivCorrErr = &NivCorrErr::MOYEN  ;
		if (donnees_bits_utilises <= getNbMotsCode(version, NivCorrErr::MOYEN_PLUS) * 8)  nouveauNivCorrErr = &NivCorrErr::MOYEN_PLUS;
		if (donnees_bits_utilises <= getNbMotsCode(version, NivCorrErr::HAUT    ) * 8)  nouveauNivCorrErr = &NivCorrErr::HAUT    ;
	}

	// Crée la chaîne de bits de données en concaténant tous les segments
	int dataCapacityBits = getNbMotsCode(version, *nouveauNivCorrErr) * 8;
	BitTampon bitTampon;
	for (size_t i = 0; i < segs.size(); i++) {
		const QAmiensRSegment &seg(segs.at(i));
		bitTampon.ajouterBits(seg.mode.modeBits, 4);
		bitTampon.ajouterBits(seg.numChars, seg.mode.indicNbBits(version));
		bitTampon.ajouterDonnees(seg);
	}

	// Ajouter un terminateur et un 'pad' à un octet, le cas échéant
	bitTampon.ajouterBits(0, std::min(4, dataCapacityBits - bitTampon.obtenirLongueurBit()));
	bitTampon.ajouterBits(0, (8 - bitTampon.obtenirLongueurBit() % 8) % 8);

	// 'Pad' avec des octets de remplacement jusqu'à ce que la capacité de données soit atteinte
	for (uint8_t padByte = 0xEC; bitTampon.obtenirLongueurBit() < dataCapacityBits; padByte ^= 0xEC ^ 0x11) bitTampon.ajouterBits(padByte, 8);

	// Création du symbole du QAmiensRCode
	return QAmiensRCode(version, *nouveauNivCorrErr, bitTampon.obtenirOctets(), masque);
}


QAmiensRCodeGeneration::QAmiensRCode::QAmiensRCode(int ver, const NivCorrErr &nivCorrErreur, const std::vector<uint8_t> &donnees_mots_de_code, int masque) :
    // Initialise les champs scalaires
    version(ver),
    taille(1 <= ver && ver <= 40 ? ver * 4 + 17 : -1),  // Évite le dépassement de signature non défini
    niveauCorrectionErreur(nivCorrErreur) {

	// Vérifie les arguments
	if (ver < 1 || ver > 40 || masque < -1 || masque > 7) throw "Valeur non valide";

	std::vector<bool> rang(taille);
	for (int i = 0; i < taille; i++) {
		modules.push_back(rang);
		estFonction.push_back(rang);
	}

	// Dessine des motifs de fonction, les mots code et applique le masque
	dessinerMotifsFonction();
	const std::vector<uint8_t> allCodewords(ajouterCorrectionErreur(donnees_mots_de_code));
	dessinerMotsCles(allCodewords);
	this->masque = gererMasquageConstructeur(masque);
}


QAmiensRCodeGeneration::QAmiensRCode::QAmiensRCode(const QAmiensRCode &qamiensrcode, int masque) :
    // Copie des champs scalaires
	version(qamiensrcode.version),
	taille(qamiensrcode.taille),
	niveauCorrectionErreur(qamiensrcode.niveauCorrectionErreur) {

	// Vérfication des arguments
	if (masque < -1 || masque > 7) throw "Valeur du masque en dehors de la plage autorisée";

    // Traitee les champs de la grille
	modules = qamiensrcode.modules;
	estFonction = qamiensrcode.estFonction;

	// Masquage de l'indicateur
	appliquerMasque(qamiensrcode.masque);  // Enlever l'ancien masque
	this->masque = gererMasquageConstructeur(masque);
}


int QAmiensRCodeGeneration::QAmiensRCode::getMasque() const {
	return masque;
}


int QAmiensRCodeGeneration::QAmiensRCode::getModule(int x, int y) const {
	if (0 <= x && x < taille && 0 <= y && y < taille) return modules.at(y).at(x) ? 1 : 0;
	else return 0;  // Bordure blanche infinie
}


std::string QAmiensRCodeGeneration::QAmiensRCode::encoderSVG(int bordure) const {
	if (bordure < 0) throw "La bordure ne peut pas être négative";
	std::ostringstream sb;

	sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

	/*
	* La DTD du SVG :
	* 'xmlns', l'espace de nom du SVG (une page où trouver de l'information à son sujet) ;
	* 'version', la version du langage (on utilise le SVG 1.1) ;
	* 'width', la largeur du document (en pixels) à l'ouverture  ;
	* 'height', la hauteur du document (en pixels) à l'ouverture.
	* On crée donc ici un viewport de 500×500 unités, ce qui fait que chaque unité de l'élément SVG correspondra
	* à une unité du viewport. On aura donc un élément SVG dont le système de coordonnée ira de 0 à 500 en largeur
	* et de 0 à 200 en hauteur.
	* En ajoutant une notion de 'viewbox', on transforme le système de coordonnées afin qu'il aille de 0 à w à
	* l'horizontale et de 0 à h à la verticale.
	*/
	sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"500\" height=\"500\" viewBox=\"0 0 ";
	sb << (taille + bordure * 2) << " " << (taille + bordure * 2) << "\">\n";

	// Rectangle extérieur qui englobe le QAmiensRCode
	sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\" stroke-width=\"0\"/>\n";
	// Balise '<path>' qui dessine des tracés.
	// C'est l'attribut 'd' qui indique les commandes (lignes, courbes, etc.) à effectuer pour dessiner le tracé.
	sb << "\t<path d=\"";
	bool en_tete = true;
	for (int y = -bordure; y < taille + bordure; y++) {
		for (int x = -bordure; x < taille + bordure; x++) {
			if (getModule(x, y) == 1) {
				if (en_tete) en_tete = false;
				else sb << " ";
				// "M" (pour 'move to') : établit un nouveau point courant.
				sb << "M" << (x + bordure) << "," << (y + bordure);
				// La commande closepath 'z' trace une ligne du point courant vers le point spécifié
				// avec la dernière commande 'M'.
				// Les commandes 'h' et 'v' sont utilisées pour dessiner des lignes horizontales (H)
				// et verticales (V) seulement.
				// On dessine 1 pixel en longueur et en hauteur.
				sb << "h1v1h-1z";
			}
		}
	}
	sb << "\" fill=\"#000000\" stroke-width=\"0\"/>\n";
	sb << "</svg>\n";
	return sb.str();
}

sf::Texture QAmiensRCodeGeneration::QAmiensRCode::encoderSFML(int bordure) const {
	if (bordure < 0) throw "La bordure ne peut pas être négative";
    sf::Image image;
    image.create(taille + 2 * bordure, taille + 2 * bordure, sf::Color::White);

	for (int y = 0; y < taille + 2 * bordure; y++) {
		for (int x = 0; x < taille + 2 * bordure; x++) {
			if (getModule(x, y) == 1) {
                image.setPixel(x + bordure, y + bordure, sf::Color::Black);
			}
		}
	}

    sf::Texture texture;
    texture.loadFromImage(image, sf::IntRect(0, 0, taille + 2 * bordure, taille + 2 * bordure));
    return texture;
}

float QAmiensRCodeGeneration::QAmiensRCode::getEchelle(int bordure) const {
    return (float)500/(taille+2*bordure);
}


void QAmiensRCodeGeneration::QAmiensRCode::dessinerMotifsFonction() {
	// Dessiner les modèles de synchronisation horizontale et verticale
	for (int i = 0; i < taille; i++) {
		definirModuleFonction(6, i, i % 2 == 0);
		definirModuleFonction(i, 6, i % 2 == 0);
	}

	// Dessinez 3 modèles de recherche (tous les coins sauf en bas à droite ; écrase certains modules de synchronisation)
	dessinerMotifViseur(3, 3);
	dessinerMotifViseur(taille - 4, 3);
	dessinerMotifViseur(3, taille - 4);

	// Dessine les nombreux motifs d'alignement
	const std::vector<int> posAlignementPattern(getPosMotifAlignement(version));
	int numAlign = posAlignementPattern.size();
	for (int i = 0; i < numAlign; i++) {
		for (int j = 0; j < numAlign; j++) {
			if ((i == 0 && j == 0) || (i == 0 && j == numAlign - 1) || (i == numAlign - 1 && j == 0))
				continue;  // Ignorer les trois 'finder' des coins
			else
				dessinerMotifAlignement(posAlignementPattern.at(i), posAlignementPattern.at(j));
		}
	}

	// Dessine des données de configuration
	dessinerBitsFormat(0);  // Valeur masque factice, écrasé plus tard dans le constructeur
	dessinerVersion();
}


void QAmiensRCodeGeneration::QAmiensRCode::dessinerBitsFormat(int masque) {
	// Calcule le code de correction d'erreur et les bits d'emballage
	int donnees = niveauCorrectionErreur.formatBits << 3 | masque;  // nivCorrErreur c'est uint2 et masque c'est uint3
	int rem = donnees;
	for (int i = 0; i < 10; i++) rem = (rem << 1) ^ ((rem >> 9) * 0x537);
	donnees = donnees << 10 | rem;
	donnees ^= 0x5412;  // uint15

	// Dessine la première copie
	for (int i = 0; i <= 5; i++) definirModuleFonction(8, i, ((donnees >> i) & 1) != 0);

	definirModuleFonction(8, 7, ((donnees >> 6) & 1) != 0);
	definirModuleFonction(8, 8, ((donnees >> 7) & 1) != 0);
	definirModuleFonction(7, 8, ((donnees >> 8) & 1) != 0);
	for (int i = 9; i < 15; i++) definirModuleFonction(14 - i, 8, ((donnees >> i) & 1) != 0);

	// Dessine la deuxième copie
	for (int i = 0; i <= 7; i++) definirModuleFonction(taille - 1 - i, 8, ((donnees >> i) & 1) != 0);
	for (int i = 8; i < 15; i++) definirModuleFonction(8, taille - 15 + i, ((donnees >> i) & 1) != 0);
	definirModuleFonction(8, taille - 8, true);
}


void QAmiensRCodeGeneration::QAmiensRCode::dessinerVersion() {
	if (version < 7) return;

	// Calcule le code de correction d'erreur et les bits d'emballage
	int rem = version;  // version c'est uint6, entre 7 et 40
	for (int i = 0; i < 12; i++) rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
	int data = version << 12 | rem;  // uint18

	// Dessine les deux copies
	for (int i = 0; i < 18; i++) {
		bool bit = ((data >> i) & 1) != 0;
		int a = taille - 11 + i % 3, b = i / 3;
		definirModuleFonction(a, b, bit);
		definirModuleFonction(b, a, bit);
	}
}


void QAmiensRCodeGeneration::QAmiensRCode::dessinerMotifViseur(int x, int y) {
	for (int i = -4; i <= 4; i++) {
		for (int j = -4; j <= 4; j++) {
			int dist = std::max(std::abs(i), std::abs(j));  // norme infinie
			int xx = x + j, yy = y + i;
			if (0 <= xx && xx < taille && 0 <= yy && yy < taille) definirModuleFonction(xx, yy, dist != 2 && dist != 4);
		}
	}
}


void QAmiensRCodeGeneration::QAmiensRCode::dessinerMotifAlignement(int x, int y) {
	for (int i = -2; i <= 2; i++) {
		for (int j = -2; j <= 2; j++) definirModuleFonction(x + j, y + i, std::max(std::abs(i), std::abs(j)) != 1);
	}
}


void QAmiensRCodeGeneration::QAmiensRCode::definirModuleFonction(int x, int y, bool estNoir) {
	modules.at(y).at(x) = estNoir;
	estFonction.at(y).at(x) = true;
}


std::vector<uint8_t> QAmiensRCodeGeneration::QAmiensRCode::ajouterCorrectionErreur(const std::vector<uint8_t> &donnees) const {
	if (donnees.size() != static_cast<unsigned int>(getNbMotsCode(version, niveauCorrectionErreur))) throw "Argument invalide";

	// Calcule les numéros de paramètre
	int nbBlocs = NB_BLOCS_CORRECTION_ERREUR[niveauCorrectionErreur.ordinal][version];
	int totalNivCorrErr = NB_MOTSCODE_CORRECTION_ERREUR[niveauCorrectionErreur.ordinal][version];
	int longBlocNivCorrErr = totalNivCorrErr / nbBlocs;
	int nbBlocCourt = nbBlocs - getNbModulesDonnesBrutes(version) / 8 % nbBlocs;
	int longBlocCourt = getNbModulesDonnesBrutes(version) / 8 / nbBlocs;

	// Divise les données en blocs et ajoute l'erreur de correction à chaque bloc
	std::vector<std::vector<uint8_t>> blocs;
	const GenerateurReedSolomon rs(longBlocNivCorrErr);
	for (int i = 0, k = 0; i < nbBlocs; i++) {
		std::vector<uint8_t> donnees_;
		donnees_.insert(donnees_.begin(), donnees.begin() + k, donnees.begin() + (k + longBlocCourt - longBlocNivCorrErr + (i < nbBlocCourt ? 0 : 1)));
		k += donnees_.size();
		const std::vector<uint8_t> NivCorrErr(rs.getReste(donnees_));
		if (i < nbBlocCourt) donnees_.push_back(0);
		donnees_.insert(donnees_.end(), NivCorrErr.begin(), NivCorrErr.end());
		blocs.push_back(donnees_);
	}

	// Intercale (sans concaténer) les octets de chaque bloc en une seule séquence
	std::vector<uint8_t> resultat;
	for (int i = 0; static_cast<unsigned int>(i) < blocs.at(0).size(); i++) {
		for (int j = 0; static_cast<unsigned int>(j) < blocs.size(); j++) {
			// Ignore l'octet de remplissage dans les blocs courts
			if (i != longBlocCourt - longBlocNivCorrErr || j >= nbBlocCourt) resultat.push_back((unsigned char &&) blocs.at(j).at(i));
		}
	}
	return resultat;
}


void QAmiensRCodeGeneration::QAmiensRCode::dessinerMotsCles(const std::vector<uint8_t> &donnees) {
	if (donnees.size() != static_cast<unsigned int>(getNbModulesDonnesBrutes(version) / 8)) throw "Argument invalide";

	size_t i = 0;  // Index du bit dans les données
	// Fais le scan en zigzag bizarre
	for (int droite = taille - 1; droite >= 1; droite -= 2) {  // Index de la colonne de droite dans chaque paire de colonnes
		if (droite == 6) droite = 5;
		for (int vertical = 0; vertical < taille; vertical++) {  // Compteur vertical
			for (int j = 0; j < 2; j++) {
				int x = droite - j;  // Cordonnées actuelles en x
				bool versLeHaut = ((droite & 2) == 0) ^ (x < 6);
				int y = versLeHaut ? taille - 1 - vertical : vertical;  // Cordonnées actuelles en y
				if (!estFonction.at(y).at(x) && i < donnees.size() * 8) {
					modules.at(y).at(x) = ((donnees.at(i >> 3) >> (7 - (i & 7))) & 1) != 0;
					i++;
				}
				// S'il existe des bits restants (0 à 7), ils sont déjà définis sur 0 ou false ou blanc
				// lorsque la grille des modules a été initialisée
			}
		}
	}
}


void QAmiensRCodeGeneration::QAmiensRCode::appliquerMasque(int masque) {
	if (masque < 0 || masque > 7) throw "Valeur du masque impossible";
	for (int y = 0; y < taille; y++) {
		for (int x = 0; x < taille; x++) {
			bool inverse;
			switch (masque) {
				case 0:  inverse = (x + y) % 2 == 0;                    break;
				case 1:  inverse = y % 2 == 0;                          break;
				case 2:  inverse = x % 3 == 0;                          break;
				case 3:  inverse = (x + y) % 3 == 0;                    break;
				case 4:  inverse = (x / 3 + y / 2) % 2 == 0;            break;
				case 5:  inverse = x * y % 2 + x * y % 3 == 0;          break;
				case 6:  inverse = (x * y % 2 + x * y % 3) % 2 == 0;    break;
				case 7:  inverse = ((x + y) % 2 + x * y % 3) % 2 == 0;  break;
				default:  throw "Erreur d'assertion";
			}
			modules.at(y).at(x) = modules.at(y).at(x) ^ (inverse & !estFonction.at(y).at(x));
		}
	}
}


int QAmiensRCodeGeneration::QAmiensRCode::gererMasquageConstructeur(int masque) {
	if (masque == -1) {  // Choisit automatiquement le meilleur masque
		int32_t penaliteMin = INT32_MAX;
		for (int i = 0; i < 8; i++) {
			dessinerBitsFormat(i);
			appliquerMasque(i);
			int penalite = getScorePenalite();
			if (penalite < penaliteMin) {
				masque = i;
				penaliteMin = penalite;
			}
			appliquerMasque(i);  // Défait le masque grâce au XOR
		}
	}
	dessinerBitsFormat(masque);  // Écraser les anciens format de bits
	appliquerMasque(masque);  // Applique le choix de masque final
	return masque;  // L'appelant doit assigner cette valeur au champ déclaré final
}


int QAmiensRCodeGeneration::QAmiensRCode::getScorePenalite() const {
	int resultat = 0;

	// Modules adjacents dans la rangée ayant la même couleur
	for (int y = 0; y < taille; y++) {
		bool couleurX = modules.at(y).at(0);
		for (int x = 1, runX = 1; x < taille; x++) {
			if (modules.at(y).at(x) != couleurX) {
				couleurX = modules.at(y).at(x);
				runX = 1;
			} else {
				runX++;
				if (runX == 5) resultat += PENALITE_N1;
				else if (runX > 5) resultat++;
			}
		}
	}

	// Modules adjacents en colonne de même couleur
	for (int x = 0; x < taille; x++) {
		bool couleurY = modules.at(0).at(x);
		for (int y = 1, runY = 1; y < taille; y++) {
			if (modules.at(y).at(x) != couleurY) {
				couleurY = modules.at(y).at(x);
				runY = 1;
			} else {
				runY++;
				if (runY == 5) resultat += PENALITE_N1;
				else if (runY > 5) resultat++;
			}
		}
	}

	// Blocs de format 2*2 de modules de même couleur
	for (int y = 0; y < taille - 1; y++) {
		for (int x = 0; x < taille - 1; x++) {
			bool  couleur = modules.at(y).at(x);
			if (  couleur == modules.at(y).at(x + 1) &&
			      couleur == modules.at(y + 1).at(x) &&
			      couleur == modules.at(y + 1).at(x + 1))
				resultat += PENALITE_N2;
		}
	}

	// Modèle type chercheur en rangées
	for (int y = 0; y < taille; y++) {
		for (int x = 0, bits = 0; x < taille; x++) {
			bits = ((bits << 1) & 0x7FF) | (modules.at(y).at(x) ? 1 : 0);
			if (x >= 10 && (bits == 0x05D || bits == 0x5D0)) resultat += PENALITE_N3; // A besoin de 11 bits accumulés
		}
	}
	// Modèle type chercheur en colonnes
	for (int x = 0; x < taille; x++) {
		for (int y = 0, bits = 0; y < taille; y++) {
			bits = ((bits << 1) & 0x7FF) | (modules.at(y).at(x) ? 1 : 0);
			if (y >= 10 && (bits == 0x05D || bits == 0x5D0)) resultat += PENALITE_N3; // A besoin de 11 bits accumulés
		}
	}

	// Balance des modules noir et blanc
	int noir = 0;
	for (int y = 0; y < taille; y++) {
		for (int x = 0; x < taille; x++) {
			if (modules.at(y).at(x)) noir++;
		}
	}
	int total = taille * taille;
	// Trouve le plus petit k tel que noir / total soit compris entre (45+5k)% et (55+5k)%
	for (int k = 0; noir*20 < (9-k)*total || noir*20 > (11+k)*total; k++) resultat += PENALITE_N4;
	return resultat;
}


std::vector<int> QAmiensRCodeGeneration::QAmiensRCode::getPosMotifAlignement(int version) {
	if (version < 1 || version > 40) throw "Version impossible";
	else if (version == 1) return std::vector<int>();
	else {
		int nbAlign = version / 7 + 2;
		int etape;
		if (version != 32) etape = (version * 4 + nbAlign * 2 + 1) / (2 * nbAlign - 2) * 2;
		else etape = 26;

		std::vector<int> resultat;
		int taille = version * 4 + 17;
		for (int i = 0, pos = taille - 7; i < nbAlign - 1; i++, pos -= etape) resultat.insert(resultat.begin(), pos);
		resultat.insert(resultat.begin(), 6);
		return resultat;
	}
}


int QAmiensRCodeGeneration::QAmiensRCode::getNbModulesDonnesBrutes(int version) {
	if (version < 1 || version > 40) throw "Version impossible";
	int resultat = (16 * version + 128) * version + 64;
	if (version >= 2) {
		int nbAlign = version / 7 + 2;
		resultat -= (25 * nbAlign - 10) * nbAlign - 55;
		if (version >= 7) resultat -= 18 * 2;  // Récupère les informations de version
	}
	return resultat;
}


int QAmiensRCodeGeneration::QAmiensRCode::getNbMotsCode(int version, const NivCorrErr &nivCorrErreur) {
	if (version < 1 || version > 40) throw "Version impossible";
	return getNbModulesDonnesBrutes(version) / 8 - NB_MOTSCODE_CORRECTION_ERREUR[nivCorrErreur.ordinal][version];
}


/*---- Les constantes ----*/

const int QAmiensRCodeGeneration::QAmiensRCode::PENALITE_N1 = 3;
const int QAmiensRCodeGeneration::QAmiensRCode::PENALITE_N2 = 3;
const int QAmiensRCodeGeneration::QAmiensRCode::PENALITE_N3 = 40;
const int QAmiensRCodeGeneration::QAmiensRCode::PENALITE_N4 = 10;


const int16_t QAmiensRCodeGeneration::QAmiensRCode::NB_MOTSCODE_CORRECTION_ERREUR[4][41] = {
	// Version: (l'index 0 c'est pour le remplissage et il est mis sur une valeur interdite)
	//0,  1,  2,  3,  4,  5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40    Niveau correction erreur
	{-1,  7, 10, 15, 20, 26,  36,  40,  48,  60,  72,  80,  96, 104, 120, 132, 144, 168, 180, 196, 224, 224, 252, 270, 300,  312,  336,  360,  390,  420,  450,  480,  510,  540,  570,  570,  600,  630,  660,  720,  750},  // BAS
	{-1, 10, 16, 26, 36, 48,  64,  72,  88, 110, 130, 150, 176, 198, 216, 240, 280, 308, 338, 364, 416, 442, 476, 504, 560,  588,  644,  700,  728,  784,  812,  868,  924,  980, 1036, 1064, 1120, 1204, 1260, 1316, 1372},  // MOYEN
	{-1, 13, 22, 36, 52, 72,  96, 108, 132, 160, 192, 224, 260, 288, 320, 360, 408, 448, 504, 546, 600, 644, 690, 750, 810,  870,  952, 1020, 1050, 1140, 1200, 1290, 1350, 1440, 1530, 1590, 1680, 1770, 1860, 1950, 2040},  // MOYEN_HAUT
	{-1, 17, 28, 44, 64, 88, 112, 130, 156, 192, 224, 264, 308, 352, 384, 432, 480, 532, 588, 650, 700, 750, 816, 900, 960, 1050, 1110, 1200, 1260, 1350, 1440, 1530, 1620, 1710, 1800, 1890, 1980, 2100, 2220, 2310, 2430},  // HAUT
};

const int8_t QAmiensRCodeGeneration::QAmiensRCode::NB_BLOCS_CORRECTION_ERREUR[4][41] = {
	// Version: (l'index 0 c'est pour le remplissage et il est mis sur une valeur interdite)
	//0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40    Niveau correction erreur
	{-1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4,  4,  4,  4,  4,  6,  6,  6,  6,  7,  8,  8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25},  // BAS
	{-1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5,  5,  8,  9,  9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49},  // MOYEN
	{-1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8,  8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68},  // MOYEN_HAUT
	{-1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81},  // HAUT
};


QAmiensRCodeGeneration::QAmiensRCode::GenerateurReedSolomon::GenerateurReedSolomon(int degre) :
		coefficients() {
	if (degre < 1 || degre > 255) throw "Degré impossible";

	// On commence avec le mononome x^0
	coefficients.resize(degre);
	coefficients.at(degre - 1) = 1;

	// Calcule le geénérateur polynomial (x - r^0) * (x - r^1) * (x - r^2) * ... * (x - r^{degre-1}),
	// dépose le plus grand terme et stocke le reste des coefficients dans l'ordre décroissant des puissances.
	int racine = 1;
	for (int i = 0; i < degre; i++) {
		// Multiplie le produit actuel par (x - r^i)
		for (size_t j = 0; j < coefficients.size(); j++) {
			coefficients.at(j) = multiplierCommeUnCommuniste(coefficients.at(j), static_cast<uint8_t>(racine)); //static_cast sert à faire une conversion en uint8_t
			if (j + 1 < coefficients.size()) coefficients.at(j) ^= coefficients.at(j + 1);
		}
		racine = (racine << 1) ^ ((racine >> 7) * 0x11D);  // Multiplie par 0x02 mod GF(2^8/0x11D)
	}
}


std::vector<uint8_t> QAmiensRCodeGeneration::QAmiensRCode::GenerateurReedSolomon::getReste(const std::vector<uint8_t> &donnees) const {
	// Calcule le reste avec une division polynomiale
	std::vector<uint8_t> resultat(coefficients.size());
	for (size_t i = 0; i < donnees.size(); i++) {
		uint8_t facteur = donnees.at(i) ^ resultat.at(0);
		resultat.erase(resultat.begin());
		resultat.push_back(0);
		for (size_t j = 0; j < resultat.size(); j++) resultat.at(j) ^= multiplierCommeUnCommuniste(coefficients.at(j), facteur);
	}
	return resultat;
}


uint8_t QAmiensRCodeGeneration::QAmiensRCode::GenerateurReedSolomon::multiplierCommeUnCommuniste(uint8_t x, uint8_t y) {
	// Multiplication dite russe
	int z = 0;
	for (int i = 7; i >= 0; i--) {
		z = (z << 1) ^ ((z >> 7) * 0x11D);
		z ^= ((y >> i) & 1) * x;
	}
	return static_cast<uint8_t>(z);
}
