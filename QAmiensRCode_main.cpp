#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "QAmiensRCode.hpp"

#include <SFML/Graphics.hpp>

// La boucle principale du programme, exécutée la première.
int main(int argc, char **argv) {

    std::string texteString;
    std::cout << "Entrer le texte à encoder en QAmiensRCode : ";
    std::cin >> texteString;    // On entre le texte qu'on veut convertir
    int correction;
    std::cout << "Entrez le niveau de correction (1 = bas, 2 = moyen, 3 = moyen_plus, 4 = haut) : ";
    std::cin >> correction;


    const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur;
    if(correction == 1) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::BAS;
    else if (correction == 2) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::MOYEN;
    else if (correction == 3) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::MOYEN_PLUS;
    else if (correction == 4) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::HAUT;

    const char *texte = texteString.c_str();    // On convertit la chaine de caractère en liste de caractères (et oui c'est important !)
    // Création du QAmiensRCode et copiage des données SVG dans un fichier xml
    const QAmiensRCodeGeneration::QAmiensRCode qamiensrcode = QAmiensRCodeGeneration::QAmiensRCode::encoderTexte(texte, nivCorrErreur);
    std::fstream xml("QAmiensRCode.xml", std::ios::out | std::ios::trunc);
    if (xml) {
        xml << qamiensrcode.encoderSVG(4);
        xml.close();
    }

    sf::RenderWindow fen(sf::VideoMode(500, 500), "QAmiensRCode");

    sf::Sprite sprite;
    sf::Texture texture(qamiensrcode.encoderSFML(4));
    sprite.setTexture(texture);
    //sprite.setScale(qamiensrcode.getEchelle(4), qamiensrcode.getEchelle(4));

    while (fen.isOpen()) {
        sf::Event event;
        while (fen.pollEvent(event)) {
            if (event.type == sf::Event::Closed) fen.close();
        }

        fen.clear();
        fen.draw(sprite);
        fen.display();
    }

    return 0;
}

