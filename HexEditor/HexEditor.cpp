#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cctype>

using namespace std;

// Zentrale Klasse für Binärdateien-Bearbeitung
// Monolithische Struktur gewählt für übersichtliche Zustandsverwaltung
class HexEditor {
private:
    string filename;                      // Speichert den Namen der geladenen Datei
    vector<unsigned char> data;           // Zentrale Datenstruktur: dynamische Größe, O(1) Zugriff
    bool isLoaded;                        // Zustandsflag verhindert Operationen auf ungültigen Daten

public:
    // Konstruktor: setzt Anfangszustand
    HexEditor() {
        isLoaded = false;                 // Explizite Initialisierung für sauberen Zustand
    }

    // Datei-Lademethode mit robuster Binärdatenverarbeitung
    bool loadFile(string name) {
        filename = name;                  // Dateiname für spätere Speicheroperationen merken

        // ios::binary ist essentiell: verhindert plattformspezifische CR/LF-Transformationen
        // die bei Binärdateien zu Datenverfälschung führen würden
        ifstream file(filename, ios::binary);

        // Robuste Fehlerbehandlung vor Dateizugriff
        if (!file) {
            cout << "Datei konnte nicht geöffnet werden.\n";
            return false;
        }

        data.clear();                      // Bestehende Daten löschen für sauberen Zustand
        unsigned char byte;                // 8-Bit-Container für einzelnes Byte

        // Kernfunktion: Sequentielle Binärdatenverarbeitung
        // reinterpret_cast<char*>(&byte) - Kritischer Cast für Stream-Kompatibilität:
        // - Stream-API arbeitet mit char* (signed: -128 bis +127)
        // - Wir benötigen unsigned char für korrekte Byte-Werte (0-255)
        // - reinterpret_cast wandelt Speicheradresse typsicher um ohne Datenveränderung
        while (file.read(reinterpret_cast<char*>(&byte), 1)) {
            data.push_back(byte);          // vector auto-expansion, amortisiert O(1)
        }

        file.close();                      // Explizite Ressourcenfreigabe
        isLoaded = true;                   // Atomare Zustandsänderung nach erfolgreichem Laden

        cout << "Datei erfolgreich geladen.\n";
        return true;
    }

    // Hex-Dump-Visualisierung mit Standard-16-Byte-Layout
    void displayContent() {
        // Guard clause: Zustand vor kritischen Operationen prüfen
        if (!isLoaded) {
            cout << "Keine Datei geladen.\n";
            return;
        }

        // Hauptiteration: Formatierte Ausgabe nach Hex-Editor-Standard
        for (size_t i = 0; i < data.size(); i++) {

            // Adresszeile alle 16 Bytes: Standard für optimale Lesbarkeit
            if (i % 16 == 0) {
                cout << "\nAdresse "
                    << setw(6) << setfill('0')  // 6-stellige Hex-Adresse mit Nullpadding
                    << hex << i << " : ";
            }

            // Byte-Ausgabe: 2-stellig hex mit führenden Nullen für einheitliche Spaltenbreite
            cout << setw(2) << setfill('0')
                << hex << (int)data[i] << " ";  // Cast zu int vermeidet Char-Interpretation

            // ASCII-Bereich nach 16 Bytes für parallele Zeichendarstellung
            if ((i + 1) % 16 == 0) {
                cout << "  ";                   // Visuelle Trennung Hex/ASCII
                for (size_t j = i - 15; j <= i; j++) {
                    // isprint(): Sicherheitsfilter gegen Terminalkontrolle durch Steuerzeichen
                    if (isprint(data[j]))
                        cout << (char)data[j];
                    else
                        cout << ".";            // Standard-Ersatz für non-printable chars
                }
            }
        }

        cout << dec << "\n";                   // Stream-State cleanup: zurück zu Dezimalmodus
    }

    // Byte-Manipulation mit doppelter Sicherheitsvalidierung
    void editByte(size_t address, unsigned char value) {
        // Erste Validierung: Systemzustand prüfen
        if (!isLoaded) {
            cout << "Keine Datei geladen.\n";
            return;
        }

        // Zweite Validierung: Bounds-Checking verhindert Buffer-Overflow
        // size_t arithmetic schließt negative Indizes durch unsigned type aus
        if (address >= data.size()) {
            cout << "Adresse außerhalb des gültigen Bereichs.\n";
            return;
        }

        // Atomare Byte-Modifikation: vector garantiert exception safety
        data[address] = value;              // Direkter O(1) Zugriff über Index
        cout << "Byte erfolgreich geändert.\n";
    }

    // Persistierung mit Write-Back-Strategie
    bool saveFile() {
        // Zustandsvalidierung vor kritischer I/O-Operation
        if (!isLoaded) {
            cout << "Keine Datei geladen.\n";
            return false;
        }

        // Binärmodus für identische Datenrekonstruktion
        ofstream file(filename, ios::binary);

        // I/O-Fehlerbehandlung für Schreiboperationen
        if (!file) {
            cout << "Fehler beim Speichern.\n";
            return false;                   // Datenintegrität bei Fehlern erhalten
        }

        // Write-Back-Prinzip: Sequentielle Rekonstruktion der Originaldatei
        // Alle Änderungen aus Memory-Buffer persistent machen
        for (size_t i = 0; i < data.size(); i++) {
            // Symmetrischer reinterpret_cast zu loadFile() für Konsistenz
            file.write(reinterpret_cast<char*>(&data[i]), 1);
        }

        file.close();                       // Explizite Ressourcenfreigabe mit automatischem Flush
        cout << "Datei gespeichert.\n";
        return true;
    }

    // Benutzerinterface mit zustandsbasierter Ablaufsteuerung
    void runMenu() {
        int choice;

        // Hauptinteraktionsschleife: Event-driven programming pattern
        do {
            // Benutzerfreundliche Menüdarstellung
            cout << "\n1 - Datei laden\n";
            cout << "2 - Inhalt anzeigen\n";
            cout << "3 - Byte bearbeiten\n";
            cout << "4 - Datei speichern\n";
            cout << "5 - Beenden\n";
            cout << "Auswahl: ";
            cin >> choice;

            // Funktionsverteilung: Jeder Branch kapselt spezifische Verantwortlichkeit
            if (choice == 1) {
                string name;
                cout << "Dateiname: ";
                cin >> name;
                loadFile(name);                // Delegierung an spezialisierte Methode
            }
            else if (choice == 2) {
                displayContent();              // Read-only Operation ohne Parameter
            }
            else if (choice == 3) {
                size_t address;                // size_t verhindert negative Adressen
                int value;                     // int für flexible Hex-Eingabe

                cout << "Adresse (dezimal eingeben): ";
                cin >> address;

                cout << "Neuer Wert (Hex, z.B. FF): ";
                cin >> hex >> value;           // Stream-Manipulator für Hex-Parsing
                cin >> dec;                    // Stream-State-Reset für Folgeoperationen

                // Sichere Konvertierung mit explizitem Cast
                editByte(address, static_cast<unsigned char>(value));
            }
            else if (choice == 4) {
                saveFile();                    // Persistierung mit Fehlerbehandlung
            }

        } while (choice != 5);                 // Exit-Condition für sauberen Programmabschluss
    }
};

// Hauptprogramm - Einstiegspunkt und Objektlebenszyklus
int main() {
    HexEditor editor;                          // Stack-Allokation für automatisches Cleanup
    editor.runMenu();                          // Delegation an Hauptfunktionalität
    return 0;                                  // Expliziter Success-Return
}