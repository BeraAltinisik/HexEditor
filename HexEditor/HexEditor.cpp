#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cctype>

using namespace std;

// Klasse zur Verwaltung der gesamten Dateiverarbeitung
class HexEditor {
private:
    string filename;                      // Speichert den Namen der geladenen Datei
    vector<unsigned char> data;           // Enthält alle Bytes der Datei
    bool isLoaded;                        // Gibt an, ob eine Datei erfolgreich geladen wurde

public:
    // Konstruktor: setzt Anfangszustand
    HexEditor() {
        isLoaded = false;                 // Standardmäßig ist keine Datei geladen
    }

    // Methode zum Laden einer Datei im Binärmodus
    bool loadFile(string name) {
        filename = name;                  // Dateiname speichern

        // Datei im Binärmodus öffnen (ios::binary verhindert Textkonvertierungen)
        ifstream file(filename, ios::binary);

        // Fehlerprüfung: Datei konnte nicht geöffnet werden
        if (!file) {
            cout << "Datei konnte nicht geöffnet werden.\n";
            return false;
        }

        data.clear();                      // Vorherige Daten löschen, falls Datei bereits geladen war
        unsigned char byte;                // Temporärer Speicher für das eingelesene Byte

        // Byte für Byte einlesen und in den Vektor speichern
        while (file.read(reinterpret_cast<char*>(&byte), 1)) {
            data.push_back(byte);
        }

        file.close();                       // Datei schließen
        isLoaded = true;                    // Status setzen: Datei ist geladen

        cout << "Datei erfolgreich geladen.\n";
        return true;
    }

    // Methode zur Anzeige des Datei-Inhalts als Hexdump
    void displayContent() {
        if (!isLoaded) {                    // Sicherstellen, dass eine Datei geladen ist
            cout << "Keine Datei geladen.\n";
            return;
        }

        for (size_t i = 0; i < data.size(); i++) {

            // Neue Zeile alle 16 Bytes für bessere Lesbarkeit
            if (i % 16 == 0) {
                cout << "\nAdresse "
                    << setw(6) << setfill('0')  // Hexadezimale Adresse immer 6 Stellen
                    << hex << i << " : ";
            }

            // Hexadezimale Ausgabe des Bytes
            cout << setw(2) << setfill('0')
                << hex << (int)data[i] << " ";

            // Nach 16 Bytes zusätzlich die ASCII-Darstellung ausgeben
            if ((i + 1) % 16 == 0) {
                cout << "  ";
                for (size_t j = i - 15; j <= i; j++) {
                    if (isprint(data[j]))      // Druckbare Zeichen als solche anzeigen
                        cout << (char)data[j];
                    else
                        cout << ".";           // Nicht-druckbare Zeichen als Punkt
                }
            }
        }

        cout << dec << "\n";                   // Hexadezimalanzeige beenden, zurück zu Dezimal
    }

    // Methode zum Bearbeiten eines einzelnen Bytes an einer bestimmten Adresse
    void editByte(size_t address, unsigned char value) {
        if (!isLoaded) {                       // Sicherstellen, dass eine Datei geladen ist
            cout << "Keine Datei geladen.\n";
            return;
        }

        if (address >= data.size()) {          // Prüfen, ob die Adresse innerhalb des gültigen Bereichs liegt
            cout << "Adresse außerhalb des gültigen Bereichs.\n";
            return;
        }

        data[address] = value;                 // Byte im Vektor ändern
        cout << "Byte erfolgreich geändert.\n";
    }

    // Methode zum Speichern der Datei zurück auf die Festplatte
    bool saveFile() {
        if (!isLoaded) {                       // Sicherstellen, dass eine Datei geladen ist
            cout << "Keine Datei geladen.\n";
            return false;
        }

        // Datei im Binärmodus öffnen
        ofstream file(filename, ios::binary);

        if (!file) {                           // Fehlerprüfung: Datei konnte nicht geschrieben werden
            cout << "Fehler beim Speichern.\n";
            return false;
        }

        // Alle Bytes aus dem Vektor zurückschreiben
        for (size_t i = 0; i < data.size(); i++) {
            file.write(reinterpret_cast<char*>(&data[i]), 1);
        }

        file.close();                          // Datei schließen
        cout << "Datei gespeichert.\n";
        return true;
    }

    // Menü zur Benutzersteuerung
    void runMenu() {
        int choice;

        do {
            // Menüoptionen ausgeben
            cout << "\n1 - Datei laden\n";
            cout << "2 - Inhalt anzeigen\n";
            cout << "3 - Byte bearbeiten\n";
            cout << "4 - Datei speichern\n";
            cout << "5 - Beenden\n";
            cout << "Auswahl: ";
            cin >> choice;

            if (choice == 1) {
                string name;
                cout << "Dateiname: ";
                cin >> name;
                loadFile(name);                // Datei laden
            }
            else if (choice == 2) {
                displayContent();              // Dateiinhalt anzeigen
            }
            else if (choice == 3) {
                size_t address;
                int value;

                cout << "Adresse (dezimal eingeben): ";
                cin >> address;                // Adresse eingeben

                cout << "Neuer Wert (Hex, z.B. FF): ";
                cin >> hex >> value;           // Hexadezimalwert eingeben
                cin >> dec;                    // wieder auf Dezimal zurückstellen

                editByte(address, static_cast<unsigned char>(value)); // Byte ändern
            }
            else if (choice == 4) {
                saveFile();                    // Datei speichern
            }

        } while (choice != 5);                 // Menü wiederholen, bis der Benutzer Beenden wählt
    }
};

// Hauptprogramm
int main() {
    HexEditor editor;                           // Objekt der HexEditor-Klasse erstellen
    editor.runMenu();                            // Menü starten
    return 0;
}
