#include <iostream>

int main() {
    std::string mode = "byte";

    std::string entree;
    std::cout << "Entr" << "\x82" << "e \x85" << " encoder en QrCode : ";
    std::cin >> entree;

    char niveau_correction_erreur;
    std::cout << "Niveau de correction d'erreurs\n--------\nL = 7%\nM = 15%\nQ = 25%\nH = 30%\n--------\n(L/M/Q/H) : ";
    std::cin >> niveau_correction_erreur;

    int nombre_caracteres_entree = entree.size();
    std::string versions [160][3];
    versions [0][0] = "17";
    versions [0][1] = "L";
    versions [0][2] = "1";
    versions [1][0] = "14";
    versions [1][1] = "M";
    versions [1][2] = "1";
}

