/*
*********************************************************************************************************************************
CE PROGRAMME NE TOURNE PAS !
CE N'EST PAS UN MOTEUR !
*********************************************************************************************************************************
*/

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "QAmiensRCode.hpp"

// La boucle principale du programme, exécutée la première.
int main(int argc, char **argv) {
    std::string texteString;
    std::cout << "Entrer le texte à encoder en QAmiensRCode : ";
    std::cin >> texteString;    // On entre le texte qu'on veut convertir
    unsigned int correction, bordure;
    std::cout << "Entrer le niveau de correction (1 = bas, 2 = moyen, 3 = haut, 4 = extrême) : ";
    std::cin >> correction;
    std::cout << "Entrer la taille de la bordure (en pixels) : ";
    std::cin >> bordure;

    const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::BAS;
    if(correction==1) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::BAS;
    if(correction==2) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::MOYEN;
    if(correction==3) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::MOYEN_PLUS;
    if(correction==4) const QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr &nivCorrErreur = QAmiensRCodeGeneration::QAmiensRCode::NivCorrErr::HAUT;

    const char *texte = texteString.c_str();    // On convertit la chaine de caractère en liste de caractères (et oui c'est important !)
    // Création du QAmiensRCode et copiage des données SVG dans un fichier xml
    const QAmiensRCodeGeneration::QAmiensRCode qamiensrcode = QAmiensRCodeGeneration::QAmiensRCode::encoderTexte(texte, nivCorrErreur);
    std::fstream xml("QAmiensRCode.xml", std::ios::out | std::ios::trunc);
    if (xml) {
        xml << qamiensrcode.encoderSVG(bordure);
        xml.close();
    }

    sf::RenderWindow fen(sf::VideoMode(500, 500), "QAmiensRCode");

    sf::Sprite sprite;
    sf::Texture texture(qamiensrcode.encoderSFML(bordure));
    sprite.setTexture(texture);
    float echelle = qamiensrcode.getEchelle(bordure);
    sprite.setScale(echelle, echelle);

    while (fen.isOpen()) {
        sf::Event event;
        while (fen.pollEvent(event)) {
            if (event.type == sf::Event::Closed) fen.close();
            // Si on appuie sur Entrer on ferme la fenêtre
            if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Return) fen.close();
        }

        fen.clear();
        fen.draw(sprite);
        fen.display();
    }

    return 0;
}

