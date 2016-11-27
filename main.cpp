#include <iostream>
#include <cstdlib>
#include <bitset>
#include <cstdio>
#include <cstring>
#include <cmath>

using namespace std;

string convertir_binaire(int nombre_decimal);

int main() {
    string mode = "byte";

    string entree;
    cout << "Entr" << "\x82" << "e \x85" << " encoder en QrCode : ";
    getline(cin, entree);

    char niveau_correction_erreur = 'H';

    int versions[40];
    versions[0] = 7;
    versions[1] = 14;
    versions[2] = 24;
    versions[3] = 34;
    versions[4] = 44;
    versions[5] = 58;
    versions[6] = 64;
    versions[7] = 84;
    versions[8] = 98;
    versions[9] = 119;
    versions[10] = 137;
    versions[11] = 155;
    versions[12] = 177;
    versions[13] = 194;
    versions[14] = 220;
    versions[15] = 250;
    versions[16] = 280;
    versions[17] = 310;
    versions[18] = 338;
    versions[19] = 382;
    versions[20] = 403;
    versions[21] = 439;
    versions[22] = 461;
    versions[23] = 511;
    versions[24] = 535;
    versions[25] = 593;
    versions[26] = 625;
    versions[27] = 658;
    versions[28] = 698;
    versions[29] = 742;
    versions[30] = 790;
    versions[31] = 842;
    versions[32] = 898;
    versions[33] = 958;
    versions[34] = 983;
    versions[35] = 1051;
    versions[36] = 1093;
    versions[37] = 1139;
    versions[38] = 1219;
    versions[39] = 1273;

    int nombre_caracteres_entree = entree.size();
    int j = 0, version;
    bool version_trouvee = false;
    while (!version_trouvee) {
        for (int i(0); i < 40; i++) {
            if (versions[i] == nombre_caracteres_entree + j) {
                version_trouvee = true;
                version = i + 1;
                break;
            }
        }
        j++;
    }

    string indicateur_de_mode = "0100";

    string indicateur_de_nombre_de_caracteres;
    if (version <= 9)
        indicateur_de_nombre_de_caracteres = std::bitset<8>(nombre_caracteres_entree).to_string();
    else
        indicateur_de_nombre_de_caracteres = std::bitset<16>(nombre_caracteres_entree).to_string();

    cout << "Mode de codage : mode byte" << "\nNombre de caract\x8Ares : " << nombre_caracteres_entree
         << "\nNiveau de correction d'erreurs : " << niveau_correction_erreur << "\nVersion : " << version
         << "\nIndicateur de mode : " << indicateur_de_mode << "\nIndicateur de nombre de caract\x8Ares : "
         << indicateur_de_nombre_de_caracteres << endl;

    string donnees_encodees;
    for (char &caractere : entree) {
        donnees_encodees += std::bitset<8>(caractere).to_string();
    }

    cout << "Donn" << "\x82" << "es encod" << "\x82" << "es : " << donnees_encodees << endl;

    string chaine = donnees_encodees + indicateur_de_nombre_de_caracteres + indicateur_de_mode;

    string terminateur;

    int nombre_bits_requis = (versions[version-1] + 2) * 8;
    cout << "Nombre de bits requis : " << nombre_bits_requis << endl;

    if (nombre_bits_requis - chaine.size() <= 4) {
        for (int i = chaine.size(); i = nombre_bits_requis; i++) {
            terminateur += "0";
        }
    } else {
        terminateur = "0000";
    }

    cout << "Terminateur : " << terminateur << endl;

    chaine += terminateur;

    int c = 0;
    while (chaine.size() % 8 != 0) {
        chaine += "0";
        c++;
    }

    cout << "Ajout de 0 n" << "\x82" << "cessaires pour obtenir une chaine de longueur multiple de 8 : " << c << endl;

    string octets_pad;
    int ajout_octets_pad = 0;
    if (nombre_bits_requis - chaine.size() > 0) {
        ajout_octets_pad = (nombre_bits_requis - chaine.size()) / 8;
        for (int i = 0; i < ajout_octets_pad; i++) {
            if(i%2==0)
                octets_pad += "111011000"; //11101100 et 00010001 sont les octets "Pad"
            else
                octets_pad += "00010001";
        }
    }

    cout << "Octets 'Pad' " << "\x85" << " ajouter pour arriver au nombre de bits requis : " << ajout_octets_pad << endl;

    chaine += octets_pad;

    cout << "Chaine finale : " << chaine << " (nombre de bits : " << chaine.size() << ")" << endl;

    system("PAUSE");

    return 0;
}