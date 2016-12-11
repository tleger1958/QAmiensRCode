#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "QAmiensRCode.hpp"


// La boucle principale du programme, exécutée la première.
int main(int argc, char **argv) {

    std::string texteString;
    std::cin >> texteString;    // On entre le texte qu'on veut convertir
    const char *texte = texteString.c_str();    // On convertit la chaine de caractère en liste de caractères (et oui c'est important !)
    const QAmiensRCodeGeneration::QAmiensRCode::Ecc &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::Ecc::BAS;  // Niveau de correction d'erreur
    // Création du QAmiensRCode et copiage des données SVG dans un fichier xml
    const QAmiensRCodeGeneration::QAmiensRCode qr = QAmiensRCodeGeneration::QAmiensRCode::encoderTexte(texte, nivCorrErreur);
    std::fstream xml("QAmiensRCode.xml", std::ios::out | std::ios::trunc);
    if (xml) {
        xml << qr.toSvgString(4);
        xml.close();
    }

    return 0;
}

