# cVisa - Modern C++ Wrapper for NIVISA

## Description

Une bibliothèque moderne C++23 qui encapsule l'API NIVISA avec un pattern Factory et du polymorphisme statique utilisant `std::variant` et `std::visit`.

## Caractéristiques

### ✅ Architecture
- **Factory Pattern** : Création automatique du bon protocole selon la configuration
- **Polymorphisme Statique** : Utilisation de `std::variant` pour zero-cost abstraction
- **Type-Safe** : Vérification des types à la compilation
- **Modern C++** : C++23 avec designated initializers

### ✅ Protocoles Supportés
1. **USB** - Communication via USB-TMC
2. **Ethernet (ETH)** - Communication TCP/IP avec VXI-11 ou HiSLIP
3. **Serial (COM)** - Communication série RS-232
4. **GPIB** - Communication IEEE-488

### ✅ Fonctionnalités NIVISA Implémentées

#### Pour tous les protocoles :
- `viOpenDefaultRM()` - Ouverture du Resource Manager
- `viOpen()` - Ouverture d'une session VISA
- `viClose()` - Fermeture de session
- `viRead()` - Lecture de données
- `viWrite()` - Écriture de données
- `viSetAttribute()` - Configuration d'attributs
- `viStatusDesc()` - Description des erreurs

#### Protocole USB :
- Configuration du timeout (`VI_ATTR_TMO_VALUE`)

#### Protocole COM (Serial) :
- Configuration du baudrate (`VI_ATTR_ASRL_BAUD`)
- Configuration des bits de données (`VI_ATTR_ASRL_DATA_BITS`)
- Configuration des bits de stop (`VI_ATTR_ASRL_STOP_BITS`)
- Configuration de la parité (`VI_ATTR_ASRL_PARITY`)

#### Protocole Ethernet :
- Configuration du timeout
- Configuration du caractère de terminaison (`VI_ATTR_TERMCHAR`)
- Activation de la détection de caractère terminal (`VI_ATTR_TERMCHAR_EN`)

#### Protocole GPIB :
- Configuration du timeout
- `viClear()` - Nettoyage du bus

## Utilisation

### Exemple basique

```cpp
#include "cVisa.h"

int main()
{
    // Connexion USB
    cVisa visa({
        .address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
        .timeout = 5000,
        .protocol = PROTOCOL::USB
    });

    if (visa.connect()) {
        visa.write("*IDN?");
        
        std::string response;
        visa.read(response);
        std::cout << "Instrument ID: " << response << "\n";
        
        visa.disconnect();
    }

    return 0;
}
```

### Exemples par protocole

#### USB
```cpp
cVisa visaUsb({
    .address = "USB0::0x1AB1::0x0588::DS1ZA170000000::INSTR",
    .timeout = 5000,
    .protocol = PROTOCOL::USB
});
```

#### Ethernet (TCPIP)
```cpp
cVisa visaEth({
    .address = "TCPIP0::192.168.1.100::inst0::INSTR",
    .timeout = 3000,
    .protocol = PROTOCOL::ETH
});
```

#### Serial (COM)
```cpp
cVisa visaCom({
    .address = "ASRL1::INSTR",
    .timeout = 2000,
    .protocol = PROTOCOL::COM
});
```

#### GPIB
```cpp
cVisa visaGpib({
    .address = "GPIB0::10::INSTR",
    .timeout = 4000,
    .protocol = PROTOCOL::GPIB
});
```

### Customisation après construction

```cpp
cVisa visa({...});

// Modifier le timeout après création
visa.setTimeout(10000);
uint32_t currentTimeout = visa.getTimeout();

// Modifier l'adresse
visa.setAddress("TCPIP0::192.168.1.101::inst0::INSTR");
std::string currentAddress = visa.getAddress();

// Obtenir le type de protocole
PROTOCOL proto = visa.getProtocol();
```

## API Publique

### Constructeur
```cpp
cVisa(configVISA config);
```

### Méthodes de communication
```cpp
bool connect();                          // Ouvrir la connexion
bool disconnect();                       // Fermer la connexion
bool write(const std::string& command);  // Envoyer une commande
bool read(std::string& response);        // Lire une réponse
```

### Méthodes de configuration
```cpp
void setTimeout(uint32_t timeout);       // Modifier le timeout (ms)
void setAddress(const std::string& address); // Modifier l'adresse
uint32_t getTimeout() const;             // Obtenir le timeout
std::string getAddress() const;          // Obtenir l'adresse
PROTOCOL getProtocol() const;            // Obtenir le protocole
```

## Gestion des erreurs

La bibliothèque utilise la fonction `checkVisaStatus()` pour vérifier tous les retours de l'API VISA :

```cpp
inline bool checkVisaStatus(ViStatus status, const std::string& operation) {
    if (status < VI_SUCCESS) {
        char errMsg[256];
        viStatusDesc(VI_NULL, status, errMsg);
        std::cerr << "[VISA Error] " << operation << ": " << errMsg 
                  << " (0x" << std::hex << status << std::dec << ")\n";
        return false;
    }
    return true;
}
```

Les erreurs sont automatiquement affichées avec :
- Le nom de l'opération qui a échoué
- Le message d'erreur VISA descriptif
- Le code d'erreur en hexadécimal

## Architecture interne

### Polymorphisme statique avec std::variant

```cpp
using ProtocolVariant = std::variant<UsbProtocol, ComProtocol, EthProtocol, GpibProtocol>;
```

### Visitors pour les opérations

Chaque opération est implémentée via un visitor :
- `ConnectVisitor` : Connexion
- `DisconnectVisitor` : Déconnexion
- `WriteVisitor` : Écriture
- `ReadVisitor` : Lecture
- `SetTimeoutVisitor` : Modification du timeout
- `SetAddressVisitor` : Modification de l'adresse
- `GetTimeoutVisitor` : Lecture du timeout
- `GetAddressVisitor` : Lecture de l'adresse

### Factory Pattern

```cpp
ProtocolVariant createProtocol(PROTOCOL protocol, const std::string& address, uint32_t timeout)
```

La fonction factory instancie automatiquement le bon protocole selon le paramètre `PROTOCOL`.

## Avantages de cette approche

1. **Performance** : Pas de virtual table, dispatch à la compilation (zero-cost abstraction)
2. **Type-Safe** : Le compilateur vérifie tous les types
3. **Extensible** : Facile d'ajouter de nouveaux protocoles
4. **Maintenable** : Code clair et structuré
5. **RAII** : Gestion automatique des ressources (sessions VISA)
6. **Modern C++** : Utilise les features C++17/20/23

## Dépendances

- **NIVISA** : Bibliothèque NI-VISA (visa64.lib)
- **C++ Standard** : C++23
- **CMake** : Version minimale 3.8

## Build

```bash
cmake -B build -G Ninja
cmake --build build
```

## Notes techniques

### Gestion des sessions VISA

Chaque protocole maintient :
- `m_defaultRM` : Session du Resource Manager
- `m_session` : Session de l'instrument

Les sessions sont automatiquement fermées dans les destructeurs (RAII pattern).

### Thread Safety

L'implémentation actuelle n'est **pas thread-safe**. Si vous devez utiliser la bibliothèque dans un contexte multi-thread, ajoutez des mutex appropriés.

## License

LGPL-2.1-or-later

## Auteur

Alexandre CARPENTIER
