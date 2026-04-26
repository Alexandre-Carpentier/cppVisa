# Guide d'extension - Ajout de nouveaux protocoles

## Introduction

Ce guide explique comment ajouter un nouveau protocole à la bibliothèque cVisa en utilisant l'architecture existante avec polymorphisme statique.

## Structure d'un protocole

Chaque protocole doit implémenter les méthodes suivantes :

```cpp
class MonProtocol
{
public:
    // Constructeur
    MonProtocol(const std::string& address, uint32_t timeout);
    
    // Destructeur
    ~MonProtocol();
    
    // Méthodes de communication
    bool connect();
    bool disconnect();
    bool write(const std::string& command);
    bool read(std::string& response);
    
    // Setters
    void setTimeout(uint32_t timeout);
    void setAddress(const std::string& address);
    
    // Getters
    uint32_t getTimeout() const;
    std::string getAddress() const;

private:
    std::string m_address;
    uint32_t m_timeout;
    ViSession m_session;
    ViSession m_defaultRM;
};
```

## Étapes pour ajouter un nouveau protocole

### 1. Ajouter l'énumération dans `cVisa.h`

```cpp
enum class PROTOCOL 
{
    USB=0,
    COM, 
    ETH,
    GPIB,
    MON_NOUVEAU_PROTOCOLE  // Ajouter ici
};
```

### 2. Créer la classe du protocole dans `cVisa.cpp`

```cpp
class MonNouveauProtocole
{
public:
    MonNouveauProtocole(const std::string& address, uint32_t timeout)
        : m_address(address), m_timeout(timeout), m_session(VI_NULL), m_defaultRM(VI_NULL)
    {
        std::cout << "[MON_PROTOCOLE] Initializing with address: " << address << "\n";
        ViStatus status = viOpenDefaultRM(&m_defaultRM);
        if (!checkVisaStatus(status, "viOpenDefaultRM")) {
            throw std::runtime_error("Failed to open VISA resource manager");
        }
    }

    ~MonNouveauProtocole() {
        if (m_session != VI_NULL) {
            disconnect();
        }
        if (m_defaultRM != VI_NULL) {
            viClose(m_defaultRM);
        }
    }

    bool connect() {
        if (m_session != VI_NULL) {
            std::cout << "[MON_PROTOCOLE] Already connected\n";
            return true;
        }

        ViStatus status = viOpen(m_defaultRM, const_cast<ViRsrc>(m_address.c_str()), 
                                 VI_NO_LOCK, m_timeout, &m_session);
        
        if (!checkVisaStatus(status, "viOpen")) {
            return false;
        }

        // Configuration spécifique à votre protocole
        status = viSetAttribute(m_session, VI_ATTR_TMO_VALUE, m_timeout);
        checkVisaStatus(status, "viSetAttribute VI_ATTR_TMO_VALUE");

        // Ajoutez ici d'autres configurations spécifiques
        // Exemple pour un protocole spécifique:
        // status = viSetAttribute(m_session, VI_ATTR_CUSTOM, custom_value);
        // checkVisaStatus(status, "viSetAttribute VI_ATTR_CUSTOM");

        std::cout << "[MON_PROTOCOLE] Connected to " << m_address << "\n";
        return true;
    }

    bool disconnect() {
        if (m_session == VI_NULL) {
            return true;
        }

        ViStatus status = viClose(m_session);
        m_session = VI_NULL;
        
        if (!checkVisaStatus(status, "viClose")) {
            return false;
        }

        std::cout << "[MON_PROTOCOLE] Disconnected from " << m_address << "\n";
        return true;
    }

    bool write(const std::string& command) {
        if (m_session == VI_NULL) {
            std::cerr << "[MON_PROTOCOLE] Not connected\n";
            return false;
        }

        ViUInt32 retCount;
        ViStatus status = viWrite(m_session, 
                                  reinterpret_cast<ViBuf>(const_cast<char*>(command.c_str())),
                                  static_cast<ViUInt32>(command.length()), 
                                  &retCount);

        if (!checkVisaStatus(status, "viWrite")) {
            return false;
        }

        std::cout << "[MON_PROTOCOLE] Command sent (" << retCount << " bytes): " << command << "\n";
        return true;
    }

    bool read(std::string& response) {
        if (m_session == VI_NULL) {
            std::cerr << "[MON_PROTOCOLE] Not connected\n";
            return false;
        }

        std::vector<char> buffer(4096);
        ViUInt32 retCount;
        ViStatus status = viRead(m_session, 
                                 reinterpret_cast<ViBuf>(buffer.data()),
                                 static_cast<ViUInt32>(buffer.size()), 
                                 &retCount);

        if (!checkVisaStatus(status, "viRead")) {
            return false;
        }

        response.assign(buffer.data(), retCount);
        std::cout << "[MON_PROTOCOLE] Response received (" << retCount << " bytes): " << response << "\n";
        return true;
    }

    void setTimeout(uint32_t timeout) { 
        m_timeout = timeout;
        if (m_session != VI_NULL) {
            viSetAttribute(m_session, VI_ATTR_TMO_VALUE, timeout);
        }
    }

    void setAddress(const std::string& address) { 
        m_address = address;
    }

    uint32_t getTimeout() const { return m_timeout; }
    std::string getAddress() const { return m_address; }

private:
    std::string m_address;
    uint32_t m_timeout;
    ViSession m_session;
    ViSession m_defaultRM;
};
```

### 3. Ajouter le protocole au variant

```cpp
using ProtocolVariant = std::variant<UsbProtocol, ComProtocol, EthProtocol, GpibProtocol, MonNouveauProtocole>;
```

### 4. Mettre à jour la factory

```cpp
ProtocolVariant createProtocol(PROTOCOL protocol, const std::string& address, uint32_t timeout)
{
    switch (protocol)
    {
    case PROTOCOL::USB:
        return UsbProtocol(address, timeout);
    case PROTOCOL::COM:
        return ComProtocol(address, timeout);
    case PROTOCOL::ETH:
        return EthProtocol(address, timeout);
    case PROTOCOL::GPIB:
        return GpibProtocol(address, timeout);
    case PROTOCOL::MON_NOUVEAU_PROTOCOLE:
        return MonNouveauProtocole(address, timeout);
    default:
        return UsbProtocol(address, timeout);
    }
}
```

### 5. Utiliser le nouveau protocole

```cpp
cVisa instrument({
    .address = "MON_ADRESSE::INSTR",
    .timeout = 5000,
    .protocol = PROTOCOL::MON_NOUVEAU_PROTOCOLE
});
```

## Exemples de configurations spécifiques

### Protocole avec buffer personnalisé

```cpp
bool connect() {
    // ... code d'ouverture standard ...
    
    // Configuration du buffer
    status = viSetAttribute(m_session, VI_ATTR_RD_BUF_SIZE, 65536);
    checkVisaStatus(status, "viSetAttribute VI_ATTR_RD_BUF_SIZE");
    
    status = viSetAttribute(m_session, VI_ATTR_WR_BUF_SIZE, 65536);
    checkVisaStatus(status, "viSetAttribute VI_ATTR_WR_BUF_SIZE");
    
    return true;
}
```

### Protocole avec format binaire

```cpp
bool read(std::string& response) {
    if (m_session == VI_NULL) {
        return false;
    }

    // Désactiver la détection de caractère terminal pour les données binaires
    viSetAttribute(m_session, VI_ATTR_TERMCHAR_EN, VI_FALSE);
    
    std::vector<char> buffer(65536);
    ViUInt32 retCount;
    ViStatus status = viRead(m_session, 
                             reinterpret_cast<ViBuf>(buffer.data()),
                             static_cast<ViUInt32>(buffer.size()), 
                             &retCount);

    if (!checkVisaStatus(status, "viRead")) {
        return false;
    }

    response.assign(buffer.data(), retCount);
    return true;
}
```

### Protocole avec timeout dynamique

```cpp
bool write(const std::string& command) {
    if (m_session == VI_NULL) {
        return false;
    }

    // Sauvegarder le timeout actuel
    ViAttrState oldTimeout;
    viGetAttribute(m_session, VI_ATTR_TMO_VALUE, &oldTimeout);
    
    // Utiliser un timeout court pour l'écriture
    viSetAttribute(m_session, VI_ATTR_TMO_VALUE, 1000);
    
    ViUInt32 retCount;
    ViStatus status = viWrite(m_session, 
                              reinterpret_cast<ViBuf>(const_cast<char*>(command.c_str())),
                              static_cast<ViUInt32>(command.length()), 
                              &retCount);

    // Restaurer le timeout
    viSetAttribute(m_session, VI_ATTR_TMO_VALUE, oldTimeout);

    return checkVisaStatus(status, "viWrite");
}
```

## Attributs VISA utiles

### Attributs généraux
- `VI_ATTR_TMO_VALUE` : Timeout en millisecondes
- `VI_ATTR_TERMCHAR` : Caractère de terminaison
- `VI_ATTR_TERMCHAR_EN` : Activer/désactiver la détection du caractère de terminaison
- `VI_ATTR_SEND_END_EN` : Envoyer END avec la dernière donnée

### Attributs Serial (ASRL)
- `VI_ATTR_ASRL_BAUD` : Vitesse en bauds
- `VI_ATTR_ASRL_DATA_BITS` : Nombre de bits de données (5-8)
- `VI_ATTR_ASRL_STOP_BITS` : Bits de stop
- `VI_ATTR_ASRL_PARITY` : Parité
- `VI_ATTR_ASRL_FLOW_CNTRL` : Contrôle de flux

### Attributs TCPIP
- `VI_ATTR_TCPIP_ADDR` : Adresse IP
- `VI_ATTR_TCPIP_PORT` : Port TCP
- `VI_ATTR_TCPIP_NODELAY` : Désactiver l'algorithme de Nagle
- `VI_ATTR_TCPIP_KEEPALIVE` : Keep-alive TCP

### Attributs GPIB
- `VI_ATTR_GPIB_PRIMARY_ADDR` : Adresse primaire
- `VI_ATTR_GPIB_SECONDARY_ADDR` : Adresse secondaire
- `VI_ATTR_GPIB_REN_STATE` : État de la ligne REN
- `VI_ATTR_GPIB_ATN_STATE` : État de la ligne ATN

### Attributs USB
- `VI_ATTR_USB_INTFC_NUM` : Numéro d'interface
- `VI_ATTR_USB_SERIAL_NUM` : Numéro de série
- `VI_ATTR_USB_PROTOCOL` : Protocole USB

## Bonnes pratiques

### 1. RAII (Resource Acquisition Is Initialization)
Toujours fermer les sessions dans le destructeur :

```cpp
~MonProtocol() {
    if (m_session != VI_NULL) {
        disconnect();
    }
    if (m_defaultRM != VI_NULL) {
        viClose(m_defaultRM);
    }
}
```

### 2. Vérification des erreurs
Toujours utiliser `checkVisaStatus()` :

```cpp
ViStatus status = viWrite(...);
if (!checkVisaStatus(status, "viWrite")) {
    return false;
}
```

### 3. État de connexion
Vérifier que la session est ouverte avant toute opération :

```cpp
if (m_session == VI_NULL) {
    std::cerr << "[PROTOCOL] Not connected\n";
    return false;
}
```

### 4. Logging cohérent
Utiliser des préfixes cohérents pour les messages de log :

```cpp
std::cout << "[MON_PROTOCOLE] Message\n";
```

### 5. Exception dans le constructeur
Lancer une exception si l'ouverture du Resource Manager échoue :

```cpp
if (!checkVisaStatus(status, "viOpenDefaultRM")) {
    throw std::runtime_error("Failed to open VISA resource manager");
}
```

## Exemple complet : Protocole VXI-11

Voici un exemple complet pour un protocole VXI-11 (TCP/IP avec RPC) :

```cpp
class Vxi11Protocol
{
public:
    Vxi11Protocol(const std::string& address, uint32_t timeout)
        : m_address(address), m_timeout(timeout), m_session(VI_NULL), m_defaultRM(VI_NULL)
    {
        std::cout << "[VXI11] Initializing with address: " << address << "\n";
        ViStatus status = viOpenDefaultRM(&m_defaultRM);
        if (!checkVisaStatus(status, "viOpenDefaultRM")) {
            throw std::runtime_error("Failed to open VISA resource manager");
        }
    }

    ~Vxi11Protocol() {
        if (m_session != VI_NULL) {
            disconnect();
        }
        if (m_defaultRM != VI_NULL) {
            viClose(m_defaultRM);
        }
    }

    bool connect() {
        if (m_session != VI_NULL) {
            return true;
        }

        ViStatus status = viOpen(m_defaultRM, const_cast<ViRsrc>(m_address.c_str()), 
                                 VI_NO_LOCK, m_timeout, &m_session);
        
        if (!checkVisaStatus(status, "viOpen")) {
            return false;
        }

        // Configuration VXI-11 spécifique
        status = viSetAttribute(m_session, VI_ATTR_TMO_VALUE, m_timeout);
        checkVisaStatus(status, "viSetAttribute VI_ATTR_TMO_VALUE");

        // Activer la détection de terminaison
        status = viSetAttribute(m_session, VI_ATTR_TERMCHAR_EN, VI_TRUE);
        checkVisaStatus(status, "viSetAttribute VI_ATTR_TERMCHAR_EN");

        status = viSetAttribute(m_session, VI_ATTR_TERMCHAR, '\n');
        checkVisaStatus(status, "viSetAttribute VI_ATTR_TERMCHAR");

        // Configuration du buffer
        status = viSetAttribute(m_session, VI_ATTR_RD_BUF_SIZE, 1048576); // 1 MB
        checkVisaStatus(status, "viSetAttribute VI_ATTR_RD_BUF_SIZE");

        std::cout << "[VXI11] Connected to " << m_address << "\n";
        return true;
    }

    bool disconnect() {
        if (m_session == VI_NULL) {
            return true;
        }

        ViStatus status = viClose(m_session);
        m_session = VI_NULL;
        
        if (!checkVisaStatus(status, "viClose")) {
            return false;
        }

        std::cout << "[VXI11] Disconnected from " << m_address << "\n";
        return true;
    }

    bool write(const std::string& command) {
        if (m_session == VI_NULL) {
            std::cerr << "[VXI11] Not connected\n";
            return false;
        }

        ViUInt32 retCount;
        ViStatus status = viWrite(m_session, 
                                  reinterpret_cast<ViBuf>(const_cast<char*>(command.c_str())),
                                  static_cast<ViUInt32>(command.length()), 
                                  &retCount);

        if (!checkVisaStatus(status, "viWrite")) {
            return false;
        }

        std::cout << "[VXI11] Command sent (" << retCount << " bytes)\n";
        return true;
    }

    bool read(std::string& response) {
        if (m_session == VI_NULL) {
            std::cerr << "[VXI11] Not connected\n";
            return false;
        }

        std::vector<char> buffer(1048576); // 1 MB buffer
        ViUInt32 retCount;
        ViStatus status = viRead(m_session, 
                                 reinterpret_cast<ViBuf>(buffer.data()),
                                 static_cast<ViUInt32>(buffer.size()), 
                                 &retCount);

        if (!checkVisaStatus(status, "viRead")) {
            return false;
        }

        response.assign(buffer.data(), retCount);
        std::cout << "[VXI11] Response received (" << retCount << " bytes)\n";
        return true;
    }

    void setTimeout(uint32_t timeout) { 
        m_timeout = timeout;
        if (m_session != VI_NULL) {
            viSetAttribute(m_session, VI_ATTR_TMO_VALUE, timeout);
        }
    }

    void setAddress(const std::string& address) { 
        m_address = address;
    }

    uint32_t getTimeout() const { return m_timeout; }
    std::string getAddress() const { return m_address; }

private:
    std::string m_address;
    uint32_t m_timeout;
    ViSession m_session;
    ViSession m_defaultRM;
};
```

## Conclusion

L'architecture avec `std::variant` et polymorphisme statique permet d'ajouter facilement de nouveaux protocoles tout en maintenant :
- Une performance optimale (pas de virtual table)
- La sécurité des types à la compilation
- Une interface uniforme pour tous les protocoles
- Une extensibilité facile

Il suffit de suivre le pattern existant et d'adapter les configurations VISA spécifiques à votre protocole.
