#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "QAmiensRCode.hpp"


// Propototypes des fonctions
static void creerQAmiensRCode();

// La boucle principale du programme, exécutée la première.
int main(int argc, char **argv) {
    creerQAmiensRCode();
    return 0;
}

// Création du QAmiensRCode
static void creerQAmiensRCode() {
    const char *text = "Thomas (car je suis narcissique) ! :o"; //Texte à encoder en QAmiensRCode
    const QAmiensRCodeGeneration::QAmiensRCode::Ecc &errCorLvl = QAmiensRCodeGeneration::QAmiensRCode::Ecc::LOW;  // Niveau de correction d'erreur
    // Création du QAmiensRCode et copiage des données SVG dans un fichier xml
    const QAmiensRCodeGeneration::QAmiensRCode qr = QAmiensRCodeGeneration::QAmiensRCode::encodeText(text, errCorLvl);
    std::fstream xml("QAmiensRCode.xml", std::ios::out | std::ios::trunc);
    if (xml) {
        xml << qr.toSvgString(4);
        xml.close();
    }
}

