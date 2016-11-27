#include <iostream>
#include <cstdlib>
#include <bitset>

using namespace std;

string convertir_binaire(int nombre_decimal);

int main() {
    string mode = "byte";

    string entree;
    cout << "Entr" << "\x82" << "e \x85" << " encoder en QrCode : ";
    cin >> entree;

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

    int indicateur_de_mode = 0b0100;

    string indicateur_de_nombre_de_caracteres;
    if (version <= 9)
        indicateur_de_nombre_de_caracteres = std::bitset<8>(nombre_caracteres_entree).to_string();
    else
        indicateur_de_nombre_de_caracteres = std::bitset<16>(nombre_caracteres_entree).to_string();

    cout << "Version : " << version << "\nIndicateur de nombre de caractères : " << indicateur_de_nombre_de_caracteres << endl;
    system("PAUSE");

    return 0;
}