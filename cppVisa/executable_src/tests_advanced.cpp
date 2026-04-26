/////////////////////////////////////////////////////////////////////////////
// Author:      Alexandre CARPENTIER
// Modified by:
// Created:     26/03/2026
// Copyright:   (c) Alexandre CARPENTIER
// Licence:     LGPL-2.1-or-later
/////////////////////////////////////////////////////////////////////////////

// Exemples avancés d'utilisation de cVisa avec NIVISA

#include "..\library_src\cVisa.h"
#include <iostream>
#include <thread>
#include <chrono>

// Exemple 1: Interrogation d'un oscilloscope via USB
void example_oscilloscope_usb()
{
    std::cout << "\n=== Example 1: Oscilloscope via USB ===\n";
    
    cVisa scope({
        .address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
        .timeout = 5000,
        .protocol = PROTOCOL::USB
    });

    if (!scope.connect()) {
        std::cerr << "Failed to connect to oscilloscope\n";
        return;
    }

    // Identification de l'instrument
    scope.write("*IDN?");
    std::string idn;
    scope.read(idn);
    std::cout << "Instrument: " << idn << "\n";

    // Reset
    scope.write("*RST");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Configuration du canal 1
    scope.write(":CHANnel1:DISPlay ON");
    scope.write(":CHANnel1:SCALe 1.0");
    scope.write(":CHANnel1:OFFSet 0.0");

    // Lecture de la configuration
    scope.write(":CHANnel1:SCALe?");
    std::string scale;
    scope.read(scale);
    std::cout << "Channel 1 Scale: " << scale << "\n";

    scope.disconnect();
}

// Exemple 2: Multimètre via Ethernet
void example_multimeter_ethernet()
{
    std::cout << "\n=== Example 2: Multimeter via Ethernet ===\n";
    
    cVisa dmm({
        .address = "TCPIP0::192.168.1.100::inst0::INSTR",
        .timeout = 3000,
        .protocol = PROTOCOL::ETH
    });

    if (!dmm.connect()) {
        std::cerr << "Failed to connect to multimeter\n";
        return;
    }

    // Identification
    dmm.write("*IDN?");
    std::string idn;
    dmm.read(idn);
    std::cout << "Instrument: " << idn << "\n";

    // Configuration pour mesure de tension DC
    dmm.write("CONFigure:VOLTage:DC");
    
    // Lecture de 5 mesures
    for (int i = 0; i < 5; ++i) {
        dmm.write("READ?");
        std::string measurement;
        dmm.read(measurement);
        std::cout << "Measurement " << (i+1) << ": " << measurement << " V\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    dmm.disconnect();
}

// Exemple 3: Générateur de fonctions via GPIB
void example_function_generator_gpib()
{
    std::cout << "\n=== Example 3: Function Generator via GPIB ===\n";
    
    cVisa fgen({
        .address = "GPIB0::10::INSTR",
        .timeout = 4000,
        .protocol = PROTOCOL::GPIB
    });

    if (!fgen.connect()) {
        std::cerr << "Failed to connect to function generator\n";
        return;
    }

    // Identification
    fgen.write("*IDN?");
    std::string idn;
    fgen.read(idn);
    std::cout << "Instrument: " << idn << "\n";

    // Configuration d'un signal sinusoïdal
    fgen.write("SOURce:FUNCtion SINusoid");
    fgen.write("SOURce:FREQuency 1000");      // 1 kHz
    fgen.write("SOURce:VOLTage 1.0");         // 1 V
    fgen.write("OUTPut ON");

    // Vérification de la configuration
    fgen.write("SOURce:FREQuency?");
    std::string freq;
    fgen.read(freq);
    std::cout << "Frequency: " << freq << " Hz\n";

    // Attendre un peu
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Désactiver la sortie
    fgen.write("OUTPut OFF");

    fgen.disconnect();
}

// Exemple 4: Alimentation via Serial (COM)
void example_power_supply_serial()
{
    std::cout << "\n=== Example 4: Power Supply via Serial ===\n";
    
    cVisa psu({
        .address = "ASRL1::INSTR",
        .timeout = 2000,
        .protocol = PROTOCOL::COM
    });

    if (!psu.connect()) {
        std::cerr << "Failed to connect to power supply\n";
        return;
    }

    // Identification
    psu.write("*IDN?\n");
    std::string idn;
    psu.read(idn);
    std::cout << "Instrument: " << idn << "\n";

    // Configuration de la tension et du courant
    psu.write("VOLT 5.0\n");
    psu.write("CURR 0.5\n");
    
    // Activation de la sortie
    psu.write("OUTP ON\n");

    // Lecture de la tension et du courant
    psu.write("MEAS:VOLT?\n");
    std::string voltage;
    psu.read(voltage);
    std::cout << "Voltage: " << voltage << " V\n";

    psu.write("MEAS:CURR?\n");
    std::string current;
    psu.read(current);
    std::cout << "Current: " << current << " A\n";

    // Désactiver la sortie
    psu.write("OUTP OFF\n");

    psu.disconnect();
}

// Exemple 5: Modification dynamique des paramètres
void example_dynamic_configuration()
{
    std::cout << "\n=== Example 5: Dynamic Configuration ===\n";
    
    cVisa instrument({
        .address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
        .timeout = 2000,
        .protocol = PROTOCOL::USB
    });

    std::cout << "Initial timeout: " << instrument.getTimeout() << " ms\n";
    std::cout << "Initial address: " << instrument.getAddress() << "\n";

    // Modifier le timeout pour des opérations lentes
    instrument.setTimeout(10000);
    std::cout << "New timeout: " << instrument.getTimeout() << " ms\n";

    // Potentiellement changer d'adresse (même si peu commun)
    instrument.setAddress("USB0::0x1AB1::0x0588::DS1ZA170000001::INSTR");
    std::cout << "New address: " << instrument.getAddress() << "\n";

    // Les changements sont pris en compte pour la prochaine connexion
}

// Exemple 6: Gestion d'erreurs robuste
void example_error_handling()
{
    std::cout << "\n=== Example 6: Error Handling ===\n";
    
    // Tentative de connexion à un instrument inexistant
    cVisa instrument({
        .address = "USB0::0xFFFF::0xFFFF::INVALID::INSTR",
        .timeout = 2000,
        .protocol = PROTOCOL::USB
    });

    if (!instrument.connect()) {
        std::cout << "Expected: Connection failed for invalid address\n";
    }

    // Tentative d'écriture sans connexion
    if (!instrument.write("*IDN?")) {
        std::cout << "Expected: Write failed - not connected\n";
    }

    // Tentative de lecture sans connexion
    std::string response;
    if (!instrument.read(response)) {
        std::cout << "Expected: Read failed - not connected\n";
    }
}

// Exemple 7: Communication avec plusieurs instruments
void example_multiple_instruments()
{
    std::cout << "\n=== Example 7: Multiple Instruments ===\n";
    
    // Créer plusieurs objets pour différents instruments
    cVisa scope({
        .address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
        .timeout = 5000,
        .protocol = PROTOCOL::USB
    });

    cVisa dmm({
        .address = "TCPIP0::192.168.1.100::inst0::INSTR",
        .timeout = 3000,
        .protocol = PROTOCOL::ETH
    });

    cVisa fgen({
        .address = "GPIB0::10::INSTR",
        .timeout = 4000,
        .protocol = PROTOCOL::GPIB
    });

    // Connexion à tous les instruments
    bool scopeOk = scope.connect();
    bool dmmOk = dmm.connect();
    bool fgenOk = fgen.connect();

    std::cout << "Scope connected: " << (scopeOk ? "Yes" : "No") << "\n";
    std::cout << "DMM connected: " << (dmmOk ? "Yes" : "No") << "\n";
    std::cout << "FGen connected: " << (fgenOk ? "Yes" : "No") << "\n";

    if (scopeOk && dmmOk && fgenOk) {
        // Configuration du générateur
        fgen.write("SOURce:FUNCtion SINusoid");
        fgen.write("SOURce:FREQuency 1000");
        fgen.write("OUTPut ON");

        // Mesure avec le multimètre
        dmm.write("CONFigure:VOLTage:DC");
        dmm.write("READ?");
        std::string voltage;
        dmm.read(voltage);
        std::cout << "DMM Measurement: " << voltage << "\n";

        // Capture avec l'oscilloscope
        scope.write(":RUN");
        scope.write(":TRIGger:MODE AUTO");

        // Nettoyage
        fgen.write("OUTPut OFF");
    }

    // Déconnexion automatique dans les destructeurs
}

// Point d'entrée pour les exemples
int main()
{
    std::cout << "=== cVisa NIVISA Examples ===\n";
    std::cout << "Note: Ces exemples nécessitent des instruments réels connectés.\n";
    std::cout << "Commentez/décommentez les exemples selon vos besoins.\n\n";

    // Décommentez les exemples que vous souhaitez exécuter
    // example_oscilloscope_usb();
    // example_multimeter_ethernet();
    // example_function_generator_gpib();
    // example_power_supply_serial();
    
    example_dynamic_configuration();
    example_error_handling();
    // example_multiple_instruments();

    std::cout << "\n=== All examples completed ===\n";
    return 0;
}
